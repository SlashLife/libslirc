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

#ifndef LIBSLIRC_MODULES_CONNECTION_HPP
#define LIBSLIRC_MODULES_CONNECTION_HPP

#include <atomic>
#include <memory>
#include <string_view>

#include <boost/asio/io_service.hpp>

#include "../network.hpp"
#include "../apis/connection.hpp"

namespace slirc::modules {

class connection
: apis::connection {
public:
	connection(slirc::irc &irc, std::string_view host, unsigned port, boost::asio::io_service &io_service = get_io_service());
	~connection();

	virtual void connect() override;
	virtual void disconnect() override;

	virtual void send(std::string_view data) override;

private:
	std::string host_;
	unsigned port_;
	boost::asio::io_service &io_service_;

	struct impl;
	std::shared_ptr<impl> impl_;
	std::atomic<bool> impl_alive_;
};

}

#endif //LIBSLIRC_MODULES_CONNECTION_HPP
