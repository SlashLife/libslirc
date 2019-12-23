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

#include "../../include/slirc/modules/connection.hpp"

#include <cassert>

#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include "../../include/slirc/event.hpp"
#include "../../include/slirc/irc.hpp"

struct slirc::modules::connection::impl: std::enable_shared_from_this<slirc::modules::connection::impl> {
	impl(slirc::modules::connection &connection)
	: connection_(connection)
	, connection_state_(events::on_disconnected)
	, asio_(std::make_shared<buffers>()) {
		try {
			connection_.impl_alive_ = true;
		}
		catch(...) {
			connection_.impl_alive_ = false;
			throw;
		}
	}

	~impl() {
		if (connection_state_ != events::on_disconnected) {
			shut_down();
		}
		connection_.impl_alive_ = false;
	}

	void shut_down() {
		if (connection_state_ != events::on_disconnected) {
			change_connection_state(events::on_disconnecting);
		}
		change_connection_state(events::on_disconnected);
	}

	struct shared_asio {
		buffers()
		: recv_stream_(&recv_buffer_)
		, send_stream_(&send_buffer_) {}

		boost::asio::basic_streambuf<> recv_buffer_;
		std::ostream recv_stream_;
		boost::asio::basic_streambuf<> send_buffer_;
		std::istream send_stream_;
	};
	std::shared_ptr<shared_asio> asio_;

private:
	void change_connection_state(events new_status) {
		if (new_status != connection_state_) {
			auto event = connection_.irc.make_event(new_status);
			event->push_back(events::on_connection_status_changed);
			event->post_back();
		}
	}

	slirc::modules::connection &connection_;
	events connection_state_;
};

slirc::modules::connection::connection(slirc::irc &irc, std::string_view host, unsigned port, boost::asio::io_service &io_service)
: apis::connection(irc)
, host_(host)
, port_(port)
, io_service_(io_service)
, impl_()
, impl_alive_(false) {}

slirc::modules::connection::~connection() {
	disconnect();
}

void slirc::modules::connection::connect() {
	disconnect();
	impl_ = std::make_shared<impl>(*this);
}

void slirc::modules::connection::disconnect() {
	if (auto impl = std::move(impl_)) {
		impl->shut_down();
		impl.reset();
	}
	while(impl_alive_) {
		// Thoughts about reentrancy:
		//   impl passes weak_ptrs to itself to its callbacks.
		//   Callbacks first lock the weak_ptr and return if locking fails.
		//   Also, this function will not be called from within impl, so
		//   reentrancy from the attached io_service can only happen from a
		//   different asio handler. That in turn means that none of our impls
		//   handlers is currently running, therefore none of the weak_ptrs is
		//   locked, in which case the impl_.reset() above did successfully
		//   delete the impl instance and we will drop out of this loop.
		// Therefore waiting here is safe!
		// TODO: Is this important enough for us to spend a condition_variable on it?
		std::this_thread::yield();
	}
}

void slirc::modules::connection::send(std::string_view data) {
	if (impl_) {
	}
}
