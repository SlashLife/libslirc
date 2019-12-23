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

#ifndef LIBSLIRC_IRC_HPP
#define LIBSLIRC_IRC_HPP

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <variant>

#include <boost/signals2.hpp>

#include "event_id.hpp"
#include "module.hpp"

namespace slirc {

class event;

/**
 * \brief An IRC context.
 *
 * The IRC context is the central piece of libslirc. It represents an IRC
 * connection (not in the network sense) and keeps track of loaded modules,
 * the event queue as well as event handlers.
 *
 * \warning Throughout libslirc it is assumed that each IRC context will only
 *          be handled by a single thread at a time (though multiple distinct
 *          threads may handle the same IRC context sequentially and distinct
 *          IRC contexts may be handled concurrently). Exceptions from this are
 *          specifically marked (a notable one are the event handler and event
 *          queue APIs of this class).
 */
class irc {
public:
	/**
	 * \brief Creates an empty IRC context.
	 */
	irc();

	template<typename... Packages>
	irc(Packages&&... packages)
	: irc() {
		int dummy[] = { (packages(*this), 0)... };
	}

	irc(const irc &) = delete;
	irc &operator=(const irc &) = delete;

	irc(irc &&) = delete;
	irc &operator=(irc &&) = delete;

	/**
	 * \brief Destroys the IRC context.
	 */
	~irc();



	detail::module_base *module(const std::type_index &slirc_module_type) noexcept {
		const auto it = modules_.find(slirc_module_type);
		return it != modules_.end()
			? it->second
			: nullptr;
	}

	const detail::module_base *module(const std::type_index &slirc_module_type) const noexcept {
		const auto it = modules_.find(slirc_module_type);
		return it != modules_.end()
			? it->second
			: nullptr;
	}

	template<typename Module, std::enable_if_t<std::is_pointer_v<Module>>* = nullptr>
	std::decay_t<std::remove_pointer_t<Module>> *module() noexcept {
		using DecayedModule = std::decay_t<std::remove_pointer_t<Module>>;
		detail::module_base *modbase = module(typeid(typename DecayedModule::slirc_module_type));
		return dynamic_cast<DecayedModule*>(modbase);
	}

	template<typename Module, std::enable_if_t<std::is_pointer_v<Module>>* = nullptr>
	const std::decay_t<std::remove_pointer_t<Module>> *module() const noexcept {
		using DecayedModule = std::decay_t<std::remove_pointer_t<Module>>;
		const detail::module_base *modbase = module(typeid(typename DecayedModule::slirc_module_type));
		return dynamic_cast<const DecayedModule*>(modbase);
	}

	template<typename Module, std::enable_if_t<!std::is_pointer_v<Module>>* = nullptr>
	std::decay_t<Module> &module() {
		if (const auto modptr = module<std::decay_t<Module>*>()) {
			return *modptr;
		}
		else {
			throw std::out_of_range("No matching module loaded.");
		}
	}

	template<typename Module, std::enable_if_t<!std::is_pointer_v<Module>>* = nullptr>
	const std::decay_t<Module> &module() const {
		if (const auto modptr = module<std::decay_t<Module>*>()) {
			return *modptr;
		}
		else {
			throw std::out_of_range("No matching module loaded.");
		}
	}

	template<typename Module, typename... Args>
	Module &load_module(Args&&... module_args) {
		using EffectiveModule = effective_module_implementation<Module>;
		auto &modptr = modules_[typeid(typename Module::slirc_module_type)];
		if (modptr) {
			delete modptr;
			modptr = nullptr;
		}
		Module *real_modptr = new EffectiveModule(*this, std::forward<Args>(module_args)...);
		modptr = real_modptr;
		return *real_modptr;
	};

	bool unload_module(const std::type_index &type) {
		if (
			const auto it = modules_.find(type);
			it != modules_.end()
		) {
			const auto modptr = it->second;
			modules_.erase(it);
			delete modptr;
			return true;
		}
		else {
			return false;
		}
	}

	template<typename Module>
	bool unload_module() {
		if (
			const auto it = modules_.find(typeid(typename Module::slirc_module_type));
			it != modules_.end()
			&& dynamic_cast<Module*>(it->second) != nullptr
		) {
			const auto modptr = it->second;
			modules_.erase(it);
			delete modptr;
			return true;
		}
		else {
			return false;
		}
	}



	using event_handler_signature = void(event&);
	using signal_type = boost::signals2::signal<event_handler_signature>;
	using connection_type = boost::signals2::connection;

	enum connect_position {
		at_back = boost::signals2::connect_position::at_back,
		at_front = boost::signals2::connect_position::at_front
	};

	struct slot_group
	: std::variant<signal_type::group_type, connect_position> {
		slot_group(signal_type::group_type value = 0)
		: std::variant<signal_type::group_type, connect_position>(value) {}

		slot_group(connect_position value)
		: std::variant<signal_type::group_type, connect_position>(value) {}
	};

	/**
	 * \brief Connects an event handler to an event.
	 * \tparam Func The event handlers function signature; must be callable as
	 *              <tt>f(event &, connection_type)</tt> or <tt>f(event &)</tt>.
	 *              If either is possible, the former form is chosen.
	 * \param id The event id to connect the handler to.
	 * \param f The event handler to connect to the event. Will be called with
	 *          a events matching the given event_id, and also the connection
	 *          returned from this function, if the event handler accepts a
	 *          second parameter.
	 * \param group Which group to add the event handler to.
	 * \param position Whether to add the event handler at the front or the back
	 *                 of the given group. If \c group is either \c at_back or
	 *                 \c at_front, this parameter is ignored and implicitly
	 *                 considered the same as \c group.
	 * \return A connection
	 */
	template<typename Func>
	connection_type connect(const event_id &id, Func &&f, slot_group group = at_back, connect_position position = at_back) {
		signal_type *signal;
		{ std::lock_guard<std::mutex> lock(signals_mutex_);
			signal = &signals_[id];
		}

		const auto callback =
			[f = std::forward<Func>(f)](event &ev, connection_type connection) {
				event_scoped_connection esc(ev, connection);
				if constexpr (is_callable_with_connection<Func>::value) {
					f(ev, connection);
				}
				else {
					static_assert(
						is_callable_without_connection<Func>::value,
						"passed function can neither be called with "
						"(event_t&, connection_type) nor with (event&)"
					);
					f(ev);
				}
			};

		if (group.index() == 0) {
			// grouped slot
			return signal->connect_extended(
				static_cast<boost::signals2::connect_position>(std::get<0>(group)),
				callback,
				static_cast<boost::signals2::connect_position>(position)
			);
		}
		else {
			// ungrouped slot
			return signal->connect_extended(
				callback,
				static_cast<boost::signals2::connect_position>(std::get<1>(group))
			);
		}
	}



	/**
	 * \brief Creates an event attached to the given IRC context.
	 * \param id The original event ID to initialize the event with.
	 * \return A pointer to the created event.
	 */
	std::shared_ptr<event> make_event(const event_id &id);

	/**
	 * \brief Fetches an event from the event queue.
	 * Blocks until an event becomes available, the timeout is exceeded or the
	 * destructor is called.
	 * \param timeout Maximum time to wait for an event.
	 * \return A pointer to the event fetched from the event queue or nullptr
	 *         if the timeout has exceeded or the IRC context is destructed.
	 */
	std::shared_ptr<event> fetch_event(std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

	/**
	 * \brief Posts an event to the back of the event queue.
	 * \param ev The event to add to the event queue.
	 * \note ev must belong to the exact IRC context it is posted to. Prefer
	 *          <tt>ev.post_back()</tt> instead.
	 */
	void post_event_back(event &ev);

	/**
	 * \brief Posts an event to the front of the event queue.
	 * \param ev The event to add to the event queue.
	 * \note ev must belong to the exact IRC context it is posted to. Prefer
	 *          <tt>ev.post_front()</tt> instead.
	 */
	void post_event_front(event &ev);

	/**
	 * \brief Emits an event to all event handlers registered to its \c event::current_id.
	 * \param ev The event to emit.
	 * \note To emit an event for all queued event IDs, call \c event::emit() instead.
	 */
	void emit_event(event &ev);



private:
	std::unordered_map<std::type_index, detail::module_base *> modules_;
	mutable std::mutex signals_mutex_;
		std::unordered_map<event_id, signal_type, event_id::hash> signals_; // mutable: const access may create empty signal
	mutable std::mutex event_queue_mutex_;
		std::condition_variable event_queue_condition_;
		std::vector<std::shared_ptr<event>> event_queue_front_;
		std::vector<std::shared_ptr<event>> event_queue_back_;
		std::vector<std::shared_ptr<event>>::size_type event_queue_back_skip_;
	bool shutting_down_;

	struct event_scoped_connection {
		event_scoped_connection(event &, connection_type);
		~event_scoped_connection();
		event &ev;
	};

	template<typename Func, typename=std::void_t<>>
	struct is_callable_with_connection: std::false_type {};

	template<typename Func>
	struct is_callable_with_connection<Func,
		std::void_t<decltype(
			std::declval<std::decay_t<Func>>(
				std::declval<event&>(),
				std::declval<connection_type>()
			)
		)>
	>: std::true_type {};

	template<typename Func, typename=std::void_t<>>
	struct is_callable_without_connection: std::false_type {};

	template<typename Func>
	struct is_callable_without_connection<Func,
		std::void_t<decltype(
			std::declval<std::decay_t<Func>>(
				std::declval<event&>()
			)
		)>
	>: std::true_type {};

};

}

#endif //LIBSLIRC_IRC_HPP
