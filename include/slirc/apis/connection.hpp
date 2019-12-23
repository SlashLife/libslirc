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
/// @author Simon Stienen
/// @file

#ifndef LIBSLIRC_APIS_CONNECTION_HPP
#define LIBSLIRC_APIS_CONNECTION_HPP

#include <string_view>

#include "../event_id.hpp"
#include "../module.hpp"

namespace slirc::modules {
	class connection;
}

namespace slirc::apis {

struct connection
: module<connection> {
	SLIRC_DEFAULT_IMPLEMENTATION(modules::connection);

	enum events: event_id::enum_type {
		on_connecting,
		on_connecting_failed,
		on_connected,
		on_disconnecting,
		on_disconnected,
		on_connection_status_changed,
		on_message_received
	};

	using module<connection>::module;

	virtual void connect() = 0;
	virtual void disconnect() = 0;

	virtual void send(std::string_view data) = 0;
};

}

#include "../modules/connection.hpp"

#endif //LIBSLIRC_APIS_CONNECTION_HPP
