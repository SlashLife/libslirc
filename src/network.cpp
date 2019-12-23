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

#include "../include/slirc/network.hpp"

#include <atomic>
#include <stdexcept>
#include <thread>

namespace {
	boost::asio::io_service *internal_io_service_ = nullptr;
	std::atomic<boost::asio::io_service *> active_io_service_ = nullptr;
	std::thread network_thread_;
}

slirc::network_thread::network_thread()
: io_service_() {
	if (network_thread_.joinable()) {
		throw std::logic_error(
			"slirc::network_thread: Trying to set up internal io_service, "
			"but an internal io_service exists already.\n"
			"Only one instance of slirc::network_thread may exist at any "
			"given time."
		);
	}
	network_thread_
		= std::thread([&](){

		});
	internal_io_service_ = &io_service_;

	{ boost::asio::io_service *expected = nullptr;
		// try to set active io_service to internal io_service, but only if
		// currently no active io_service exists (current == nullptr)
		active_io_service_.compare_exchange_strong(expected, internal_io_service_);
	}
}

slirc::network_thread::~network_thread() {
	internal_io_service_ = nullptr;
	{ boost::asio::io_service *expected = &io_service_;
		// try to unset active io_service, but only if the internal io_service
		// is the currently active one (current == &io_service_)
		active_io_service_.compare_exchange_strong(expected, nullptr);
	}

	io_service_.stop();
	network_thread_.join();
}

boost::asio::io_service &slirc::get_io_service() {
	auto * const active_io_service = active_io_service_.load();
	if (!active_io_service) {
		throw std::logic_error(
			"slirc::get_io_service(): No active io_service found!\n"
			"Either instantiate an slirc::network_thread or call "
			"slirc::use_external_io_service() to specify an external one."
		);
	}
	return *active_io_service;
}

void slirc::use_external_io_service(boost::asio::io_service &io_service) {
	active_io_service_ = &io_service;
}

void slirc::use_internal_io_service() {
	active_io_service_ = internal_io_service_;
}
