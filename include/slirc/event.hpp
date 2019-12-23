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

#ifndef LIBSLIRC_EVENT_HPP
#define LIBSLIRC_EVENT_HPP

#include <memory>
#include <vector>

#include "event_id.hpp"
#include "util/component_map.hpp"

namespace slirc {

class irc;

class event: public std::enable_shared_from_this<event> {
private:
	struct private_construction_tag;
public:
	typedef std::shared_ptr<event> pointer; ///< A shared pointer to an event.
	typedef std::weak_ptr<event> weak_pointer; ///< A weak pointer to an event.

	/**
	 * \brief Internal use only
	 * \sa <tt>create(slirc::irc &irc, const event_id &original_id)</tt>
	 */
	event(const private_construction_tag &, slirc::irc &, const event_id &, pointer);

	/**
	 * \brief Create a new event
	 * \param irc The IRC context the event belongs to.
	 * \param original_id The starting ID for this event.
	 * \return A smart pointer to the newly created event.
	 */
	static pointer create(slirc::irc &irc, const event_id &original_id);

	/**
	 * \brief Create a new event
	 * \param irc The IRC context the event belongs to.
	 * \param original_id The starting ID for this event.
	 * \param origin The original event the new event is spawned off.
	 * \return A smart pointer to the newly created event.
	 */
	static pointer create(slirc::irc &irc, const event_id &original_id, pointer origin);

	/**
	 * \brief Spawn new event from this event
	 * \param new_event_id The original ID for the new event.
	 * \return A smart pointer to a new event with the current and original ID
	 *         set to \c new_event_id and \c origin set to <tt>*this</tt>.
	 */
	pointer spawn(const event_id &new_event_id) {
		return create(irc, new_event_id, shared_from_this());
	}



	/**
	 * \brief Posts this event to the front of the IRC contexts event queue.
	 */
	void post_front();

	/**
	 * \brief Posts this event to the front of the IRC contexts event queue.
	 */
	void post_back();



	using underlying_type = std::vector<event_id>; ///< @brief Container type of the event_id queue
	using value_type = event_id; ///< @brief Value type of the event_id queue
	using reference = value_type &; ///< @brief Reference type of the event_id queue
	using const_reference = const value_type &; ///< @brief Constant reference type of the event_id queue
	using iterator = underlying_type::iterator; ///< @brief event_id queue iterator type
	using const_iterator = underlying_type::const_iterator; ///< @brief event_id queue const iterator type
	using reverse_iterator = underlying_type::reverse_iterator; ///< @brief event_id queue reverse iterator type
	using const_reverse_iterator = underlying_type::const_reverse_iterator; ///< @brief event_id queue const reverse iterator type

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	iterator begin();

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	iterator end();

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	const_iterator cbegin() const;

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	const_iterator cend() const;

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	const_iterator begin() const { return cbegin(); }

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	const_iterator end() const { return cend(); }

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	reverse_iterator rbegin();

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	reverse_iterator rend();

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	const_reverse_iterator crbegin() const;

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	const_reverse_iterator crend() const;

	/**
	 * \brief Returns the begin of the queue.
	 * \return An iterator to the start of the queue.
	 */
	const_reverse_iterator rbegin() const { return crbegin(); }

	/**
	 * \brief Returns the end of the queue.
	 * \return An iterator to the end of the queue.
	 */
	const_reverse_iterator rend() const { return crend(); }

	/**
	 * \brief Erases elements.
	 * \tparam Args... The types of the arguments forwarded to underlying_type::erase
	 * \param args... The arguments forwarded to underlying_type::erase
	 * \return The return value of underlying_type::erase
	 * \post Invalidates iterators as per underlying_type::erase
	 */
	template<typename... Args>
	auto erase(Args&&... args) {
		return id_queue.erase(std::forward<Args>(args)...);
	}

	/**
	 * \brief Inserts elements.
	 * \tparam Args... The types of the arguments forwarded to underlying_type::insert
	 * \param args... The arguments forwarded to underlying_type::insert
	 * \return The return value of underlying_type::insert
	 * \post Invalidates iterators as per underlying_type::insert
	 */
	template<typename... Args>
	auto insert(Args&&... args) {
		return id_queue.insert(std::forward<Args>(args)...);
	}

	/**
	 * \brief Emplaces elements.
	 * \tparam Args... The types of the arguments forwarded to underlying_type::emplace
	 * \param args... The arguments forwarded to underlying_type::emplace
	 * \return The return value of underlying_type::emplace
	 * \post Invalidates iterators as per underlying_type::emplace
	 */
	template<typename... Args>
	auto emplace(Args&&... args) {
		return id_queue.emplace(std::forward<Args>(args)...);
	}



	/**
	 * \brief Emits the event for all queued event_ids
	 * \post Invalidates all iterators
	 */
	void emit();

	/**
	 * \brief Emits the event for a specific event_id
	 * \param id The event_id to execute the event as
	 */
	void emit_as(const event_id &id);

	/**
	 * \brief Adds an event_id at the beginning of the queue (parsed next)
	 * \param id The event id to add to the beginning of the queue
	 * \post Invalidates all iterators
	 */
	void push_front(const event_id &id);

	/**
	 * \brief Adds an event_id at the end of the queue (parsed last)
	 * \param id The event id to add to the end of the queue
	 * \post Invalidates all iterators
	 */
	void push_back(const event_id &id);

	/**
	 * \brief Removes an event_id from the beginning of the queue
	 * \pre <tt>!empty()</tt>
	 * \post Invalidates all iterators
	 */
	void pop_front();

	/**
	 * \brief Removes an event_id from the end of the queue
	 * \pre <tt>!empty()</tt>
	 * \post Invalidates all iterators
	 */
	void pop_back();

	/**
	 * \brief Checks the size of the queue
	 * \return The number of queued items
	 */
	underlying_type::size_type size() const {
		return id_queue.size() + next_id_queue.size() - skipped;
	}

	/**
	 * \brief Checks whether the queue is empty
	 * \return
	 *     - \c false if the queue contains any items,
	 *     - \c true if the queue is empty
	 */
	bool empty() const {
		return id_queue.size() == skipped && next_id_queue.empty();
	}

	const pointer origin; ///< The original event this event is spawned off

	const value_type original_id; ///< The original event id this event was created as.
	const value_type &current_id; ///< The event id this event is currently being handled as.

	slirc::irc &irc; ///< The IRC context this event is associated with.

	util::component_map data; ///< The data attached to the event.

private:
	mutable underlying_type next_id_queue;
	mutable underlying_type id_queue;
	mutable underlying_type::size_type skipped;
	value_type current_id_;
};

}

#endif //LIBSLIRC_EVENT_HPP
