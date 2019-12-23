// Copyright 2018 Simon Stienen
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

#include "../include/slirc/irc.hpp"

#include "../include/slirc/event.hpp"
#include "../include/slirc/module.hpp"

slirc::irc::irc()
: modules_()
, signals_mutex_()
, signals_()
, event_queue_mutex_()
, event_queue_condition_()
, event_queue_front_()
, event_queue_back_()
, event_queue_back_skip_(0)
, shutting_down_(false) {}

slirc::irc::~irc() {
	// TODO: Allow vetoing of dependencies for orderly shutdown
	while(!modules_.empty()) {
		detail::module_base *modptr = modules_.begin()->second;
		modules_.erase(modules_.begin());
		delete modptr;
	}

	{ std::lock_guard<std::mutex> lock(event_queue_mutex_);
		shutting_down_ = true;
		event_queue_condition_.notify_all();
	}
}

void slirc::irc::emit_event(slirc::event &ev) {
	signal_type *signal;
	{ std::lock_guard<std::mutex> lock(signals_mutex_);
		signal = &signals_[ev.current_id];
	}

	(*signal)(ev);
}

std::shared_ptr<slirc::event> slirc::irc::make_event(const slirc::event_id &id) {
	return event::create(*this, id);
}

std::shared_ptr<slirc::event> slirc::irc::fetch_event(std::chrono::milliseconds timeout) {

	std::unique_lock<std::mutex> lock(event_queue_mutex_);
	if (
		event_queue_condition_.wait_for(
			lock,
			timeout,
			[&]{
				return
					shutting_down_
					|| !event_queue_front_.empty()
					|| event_queue_back_skip_ < event_queue_back_.size();
			}
		)
	) {
		return {};
	}

	std::shared_ptr<slirc::event> retval;
	if (!event_queue_front_.empty()) {
		retval = event_queue_front_.back();
		event_queue_front_.pop_back();
	}
	else if (event_queue_back_skip_ < event_queue_back_.size()) {
		retval = std::move(event_queue_back_[event_queue_back_skip_]);
		++event_queue_back_skip_;
	}
	return retval;
}

void slirc::irc::post_event_back(slirc::event &ev) {
	assert(&(ev.irc) == this && "Must post event to correct IRC context!");

	std::lock_guard<std::mutex> lock(event_queue_mutex_);

	if (
		event_queue_back_.size()/2 < event_queue_back_skip_
		&& event_queue_back_.size() == event_queue_back_.capacity()
	) {
		event_queue_back_.erase(
			std::copy(
				event_queue_back_.begin() + event_queue_back_skip_,
				event_queue_back_.end(),
				event_queue_back_.begin()
			),
			event_queue_back_.end()
		);
		event_queue_back_skip_ = 0;
	}

	event_queue_back_.push_back(ev.shared_from_this());
}

void slirc::irc::post_event_front(slirc::event &ev) {
	assert(&(ev.irc) == this && "Must post event to correct IRC context!");

	std::lock_guard<std::mutex> lock(event_queue_mutex_);
	event_queue_front_.push_back(ev.shared_from_this());
}

slirc::irc::event_scoped_connection::event_scoped_connection(slirc::event &ev, slirc::irc::connection_type connection)
: ev(ev) {
	ev.data.insert(connection);
}

slirc::irc::event_scoped_connection::~event_scoped_connection() {
	ev.data.erase<connection_type>();
}
