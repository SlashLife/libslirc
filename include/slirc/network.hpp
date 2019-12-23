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

#ifndef LIBSLIRC_NETWORK_HPP
#define LIBSLIRC_NETWORK_HPP

#include <boost/asio/io_service.hpp>

namespace slirc {

/**
 * \brief Represents a libslirc network thread.
 *
 * Some of the facilities in libslirc require a Boost.ASIO io_service for their
 * operation. This can either be passed manually on a per-instance basis, or a
 * default one will be used. The default can be set to use a user provided
 * io_service, by calling the function \c use_external_io_service(), or
 * libslirc can provide an io_service itself if a \c network_thread instance
 * is created.
 *
 * If a user provided io_service is used, it is up to the user to ensure that
 * it is regularly run. When using the \c network_thread instance, it will be
 * automatically run in the background for the lifetime of the instance.
 */
class network_thread {
	network_thread();
	network_thread(const network_thread &) = delete;
	network_thread(network_thread &&) = delete;
	~network_thread();

	boost::asio::io_service &get_io_service() const {
		return io_service_;
	}

private:
	mutable boost::asio::io_service io_service_;
};

/**
 * \brief Get the active io_service used by libslirc.
 * \return A reference to the active io_service.
 * \throws std::logic_error if no io_service is set.
 */
boost::asio::io_service &get_io_service();

void use_external_io_service(boost::asio::io_service &io_service);
void use_internal_io_service();

}

#endif //LIBSLIRC_NETWORK_HPP
