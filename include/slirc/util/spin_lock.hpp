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

#ifndef LIBSLIRC_SPIN_LOCK_HPP
#define LIBSLIRC_SPIN_LOCK_HPP

#include <atomic>

namespace slirc::util {

/// @brief A spin lock
class spin_lock {
	/**
	 * \brief Creates a spin lock
	 */
	spin_lock();
	spin_lock(spin_lock&) = delete;

	/**
	 * \brief Locks the spin lock. Will yield the thread until locking succeeds.
	 * \pre The spin lock is not locked by the calling thread
	 * \post The spin lock is locked by the calling thread
	 */
	void lock() noexcept;

	/**
	 * \brief Unlocks the spin lock
	 * \pre The spin lock is locked by the calling thread
	 * \post The spin lock is not locked by the calling thread
	 */
	void unlock() noexcept;

	/**
	 * \brief Unlocks the spin lock
	 * \return
	 *     - \c false if locking the spin lock failed,
	 *     - \c true if the spin lock was successfully locked
	 * \pre The spin lock is not locked by the calling thread
	 * \post
	 *     - When returning \c false: The spin lock is not locked by the calling thread
	 *     - When returning \c true: The spin lock is locked by the calling thread
	 */
	bool try_lock() noexcept;

	/**
	 * \brief Posts the event to the front of irc IRC contexts event queue.
	 */
	void post_front() const;

	/**
	 * \brief Posts the event to the front of irc IRC contexts event queue.
	 */
	void post_back() const;

private:
	std::atomic<bool> locked_;
};

}

#endif //LIBSLIRC_SPIN_LOCK_HPP
