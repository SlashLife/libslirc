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

#ifndef LIBSLIRC_MODULE_HPP
#define LIBSLIRC_MODULE_HPP

#include <type_traits>

namespace slirc {

class irc;

namespace detail {

class module_base {
public:
	module_base(slirc::irc &irc)
	: irc(irc) {}

	virtual ~module_base() = default;

	irc &irc;
};

}

template<typename T>
class module
: public detail::module_base {
public:
	//static_assert(std::is_base_of_v<module<T>, T>, "Error: T does not derive from module<T> CRTP!");

	using slirc_module_type = T;

	using detail::module_base::module_base;
};

#ifndef SLIRC_DEFAULT_IMPLEMENTATION
#define SLIRC_DEFAULT_IMPLEMENTATION(DefImplementationClass) \
	using slirc_default_implementation = DefImplementationClass*
#endif // SLIRC_DEFAULT_IMPLEMENTATION

namespace detail {
	template<typename T, bool IsAbstract, typename=std::void_t<>>
	struct effective_module_implementation_impl {
		using type = T;
	};

	template<typename T>
	struct effective_module_implementation_impl<T, true, std::void_t<typename T::slirc_default_implementation>> {
		using raw_type = typename T::slirc_default_implementation;
		static_assert(std::is_pointer_v<raw_type>, "Lookup type not created by SLIRC_DEFAULT_IMPLEMENTATION()");

		using type = std::remove_pointer_t<raw_type>;
		static_assert(std::is_base_of_v<T, type>, "Default implementation type found, but does not implement requested interface.");
		static_assert(!std::is_abstract_v<type>, "Default implementation type found, but is abstract itself.");
	};
}

template<typename T>
using effective_module_implementation =
	typename detail::effective_module_implementation_impl<T, std::is_abstract_v<T>>::type;

}

#endif //LIBSLIRC_MODULE_HPP
