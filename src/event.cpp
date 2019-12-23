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

#include "../include/slirc/event.hpp"

#include <iterator>
#include <c++/7.2.0/cassert>

#include "../include/slirc/irc.hpp"

namespace {
	using id_vector = std::vector<slirc::event_id>;
	void normalize(id_vector &dst, id_vector::size_type &skipped_elements, id_vector &src) {
		using reverse_iterator = std::reverse_iterator<id_vector::iterator>;
		if (skipped_elements < src.size()) {
			// need to move elements in dst to make space for src elements
			const auto old_size = dst.size();
			const auto required_additional_space = src.size() - skipped_elements;

			dst.resize(old_size + required_additional_space);
			std::copy(
				reverse_iterator(dst.begin() + old_size),
				reverse_iterator(dst.begin() + skipped_elements),
				reverse_iterator(dst.end())
			);

			std::copy(
				src.begin(),
				src.end(),
				reverse_iterator(dst.begin() + skipped_elements + required_additional_space)
			);
			skipped_elements = 0;
		}
		else {
			std::copy(
				src.begin(),
				src.end(),
				reverse_iterator(dst.begin() + skipped_elements)
			);
			skipped_elements -= src.size();
		}
		src.clear();
	}
}

struct slirc::event::private_construction_tag {};

slirc::event::event(
	const slirc::event::private_construction_tag &,
	slirc::irc &irc,
	const slirc::event_id &original_id,
	pointer origin
)
: std::enable_shared_from_this<event>()
, irc(irc)
, origin(origin)
, original_id(original_id)
, current_id(current_id_)
, next_id_queue()
, id_queue()
, skipped(0)
, current_id_(original_id) {}

slirc::event::pointer slirc::event::create(slirc::irc &irc, const slirc::event_id &original_id) {
	return std::make_shared<event>(private_construction_tag{}, irc, original_id, pointer{});
}

slirc::event::pointer slirc::event::create(slirc::irc &irc, const slirc::event_id &original_id, pointer origin) {
	return std::make_shared<event>(private_construction_tag{}, irc, original_id, origin);
}

void slirc::event::post_front() {
	irc.post_event_front(*this);
}

void slirc::event::post_back() {
	irc.post_event_back(*this);
}

slirc::event::iterator slirc::event::begin() {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.begin() + skipped;
}

slirc::event::const_iterator slirc::event::cbegin() const {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.cbegin() + skipped;
}

slirc::event::iterator slirc::event::end() {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.end();
}

slirc::event::const_iterator slirc::event::cend() const {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.cend();
}

slirc::event::reverse_iterator slirc::event::rbegin() {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.rbegin() + skipped;
}

slirc::event::const_reverse_iterator slirc::event::crbegin() const {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.crbegin() + skipped;
}

slirc::event::reverse_iterator slirc::event::rend() {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.rend();
}

slirc::event::const_reverse_iterator slirc::event::crend() const {
	normalize(id_queue, skipped, next_id_queue);
	return id_queue.crend();
}

void slirc::event::emit() {
	while((normalize(id_queue, skipped, next_id_queue), skipped < id_queue.size())) {
		current_id_ = id_queue[skipped];
		++skipped;
		irc.emit_event(*this);
	}
}

void slirc::event::emit_as(const slirc::event_id &id) {
	const slirc::event_id old_id = current_id_;
	current_id_ = id;
	try {
		irc.emit_event(*this);
		current_id_ = old_id;
	}
	catch(...) {
		current_id_ = old_id;
		throw;
	}
}

void slirc::event::push_back(const slirc::event_id &id) {
	if (
		id_queue.size() == id_queue.capacity()
		&& skipped != 0
	) {
		// move existing elements to front of vector rather than reallocating
		id_queue.erase(
			std::copy(
				id_queue.begin() + skipped,
				id_queue.end(),
				id_queue.begin()
			),
			id_queue.end()
		);
	}
	id_queue.push_back(id);
}

void slirc::event::push_front(const slirc::event_id &id) {
	next_id_queue.push_back(id);
}

void slirc::event::pop_back() {
	assert((skipped < id_queue.size() || !next_id_queue.empty()) && "needs to contain an element to pop");
	if (id_queue.size() == skipped) {
		normalize(id_queue, skipped, next_id_queue);
	}
	id_queue.pop_back();
}

void slirc::event::pop_front() {
	if (!next_id_queue.empty()) {
		next_id_queue.pop_back();
	}
	else {
		assert(skipped < id_queue.size() && "needs to contain an element to pop");
		++skipped;
	}
}
