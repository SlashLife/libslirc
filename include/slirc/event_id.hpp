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

#ifndef LIBSLIRC_EVENT_ID_HPP
#define LIBSLIRC_EVENT_ID_HPP

#include <functional>
#include <tuple>
#include <typeindex>

namespace slirc {

struct event_id
: std::tuple<std::type_index, unsigned> {
	using enum_type = unsigned;
	using std::tuple<std::type_index, enum_type>::tuple;

	/// @brief Hash type for event_id
	struct hash {
		std::size_t operator()(const event_id &value) const {
			return
				std::hash<std::type_index>{}(std::get<0>(value))
				^ std::hash<unsigned>{}(std::get<1>(value));
		}
	};

	constexpr event_id()
	: std::tuple<std::type_index, enum_type>{
		typeid(none_t),
		static_cast<enum_type>(none_t::NONE)
	} {}

	template<
		typename Enum,
		std::enable_if_t<
			std::is_enum_v<std::decay_t<Enum>>
		>* = nullptr
	>
	constexpr event_id(Enum id)
	: std::tuple<std::type_index, enum_type>{
		typeid(std::decay_t<Enum>),
		static_cast<enum_type>(id)
	} {
		static_assert(
			std::is_same_v<
				std::underlying_type_t<std::decay_t<Enum>>,
				enum_type
			>,
			"Trying to instantiate slirc::event_id from incompatible enum type. "
			"(nums must have underlying type slirc::event_id::enum_type.)"
		);
	}

	constexpr bool operator==(const event_id &other) const {
		return
			static_cast<const std::tuple<std::type_index, enum_type> &>(*this)
			== static_cast<const std::tuple<std::type_index, enum_type> &>(other);
	}

	constexpr bool operator!=(const event_id &other) const {
		return
			static_cast<const std::tuple<std::type_index, enum_type> &>(*this)
			!= static_cast<const std::tuple<std::type_index, enum_type> &>(other);
	}

private:
	enum class none_t: enum_type { NONE };
};

}

#endif //LIBSLIRC_EVENT_ID_HPP
