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

#ifndef LIBSLIRC_COMPONENT_MAP_HPP
#define LIBSLIRC_COMPONENT_MAP_HPP

#include <cassert>

#include <any>
#include <typeindex>
#include <unordered_map>

namespace slirc::util {

/// \brief Container type using types as keys, able to hold 0 or 1 instances per type.
class component_map {
public:
	using underlying_type = std::unordered_map<std::type_index, std::any>;
	using size_type = underlying_type::size_type;

	/**
	 * \brief Fetches an element from the map
	 * \tparam T The type to look up. This may be cv-qualified to change the type of reference returned.
	 * \return A reference to the stored object
	 * \throw std::out_of_range if no object of the given type is stored
	 */
	template<typename T>
	T &at() {
#ifndef NDEBUG
		try {
#endif
			return std::any_cast<T&>(content_.at(typeid(std::decay_t<T>)));
#ifndef NDEBUG
		}
		catch(std::bad_any_cast &) {
			assert(false && "type_index lookup should have guaranteed no std::bad_any_cast");
			throw;
		}
#endif
	}

	/**
	 * \brief Fetches an element from the map
	 * \tparam T The type to look up. This may be cv-qualified to change the type of reference returned.
	 *           (The const qualification is effectively ignored in this overload.)
	 * \return A const reference to the stored object
	 * \throw std::out_of_range if no object of the given type is stored
	 */
	template<typename T>
	std::add_const_t<T> &at() const {
#ifndef NDEBUG
		try {
#endif
			return std::any_cast<T&>(content_.at(typeid(std::decay_t<T>)));
#ifndef NDEBUG
		}
		catch(std::bad_any_cast &) {
			assert(false && "type_index lookup should have guaranteed no std::bad_any_cast");
			throw;
		}
#endif
	}

	/**
	 * \brief Emplaces an object into the map, if none exists yet.
	 * \tparam T The type of the object to add
	 * \tparam Args... The \c T constructor argument types.
	 * \param args... The arguments passed to the \c T constructor.
	 * \return A reference to the existing or newly inserted object.
	 */
	template<typename T, typename... Args>
	std::decay_t<T> &at_or_emplace(Args&&... args) {
		return (content_.end() != content_.find(typeid(std::decay_t<T>)))
			? at<T>()
			: emplace<T>(std::forward<Args>(args)...);
	};

	/**
	 * \brief Emplaces an object into the map.
	 * \tparam T The type of the object to add
	 * \tparam Args... The \c T constructor argument types.
	 * \param args... The arguments passed to the \c T constructor.
	 * \return A reference to the inserted object.
	 *
	 * \note If an object already exists, a new one will be created nontheless,
	 *       potentially invalidating references to the former element.
	 */
	template<typename T, typename... Args>
	std::decay_t<T> &emplace(Args&&... args) {
		std::any temp_any;
		std::any &map_any = content_[typeid(std::decay_t<T>)];
		temp_any.swap(map_any);

		try {
			return map_any.emplace<std::decay_t<T>>(std::forward<Args>(args)...);
		}
		catch(...) {
			if (temp_any.has_value()) {
				// put old value back
				temp_any.swap(map_any);
			}
			else {
				// old value was empty; remove from map
				content_.erase(typeid(std::decay_t<T>));
			}
			throw;
		}
	}

	/**
	 * \brief Emplaces an object into the map.
	 * \tparam T The type of the object to add
	 * \param value The object to insert
	 * \return A reference to the inserted object.
	 *
	 * \note If an object already exists, a new one will be created nontheless,
	 *       potentially invalidating references to the former element.
	 */
	template<typename T>
	std::decay_t<T> &insert(const T &value) {
		return emplace<T>(value);
	}

	/**
	 * \brief Emplaces an object into the map.
	 * \tparam T The type of the object to add
	 * \param value The object to insert
	 * \return A reference to the inserted object.
	 *
	 * \note If an object already exists, a new one will be created nontheless,
	 *       potentially invalidating references to the former element.
	 */
	template<typename T>
	std::decay_t<T> &insert(const T &&value) {
		return emplace<T>(std::move(value));
	}

	/**
	 * \brief Removes an object from the map.
	 * \tparam T The type of the object to remove.
	 * \return
	 *     - \c false if no object of the given type was stored,
	 *     - \c true if an object was removed.
	 * \post <tt>.at\<T\>()</tt> will throw
	 */
	template<typename T>
	bool erase() {
		content_.erase(typeid(std::decay_t<T>));
	}

	/**
	 * \brief Checks whether the \c component_map is empty.
	 * \return
	 *     - \c false if any objects are stored,
	 *     - \c true if no objects are stored
	 */
	bool empty() const {
		return content_.empty();
	}

	/**
	 * \brief Checks the number of elements stored in the \c component_map
	 * \return The number of elements stored.
	 */
	size_type size() const {
		return content_.size();
	}

private:
	underlying_type content_;
};

}

#endif //LIBSLIRC_COMPONENT_MAP_HPP
