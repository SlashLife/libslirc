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

#ifndef LIBSLIRC_LOAD_MODULE_HPP
#define LIBSLIRC_LOAD_MODULE_HPP

#include <tuple>
#include <utility>

#include "../irc.hpp"

namespace slirc::packages {

namespace detail {
	template<typename Module, typename PackedArgs, typename Indices>
	class load_module_package;

	template<typename Module, typename PackedArgs, std::size_t... Indices>
	class load_module_package<Module, PackedArgs, std::index_sequence<Indices...>> {
		load_module_package(PackedArgs &&args)
		: args_(std::move(args)) {}

		void operator()(irc &context) {
			context.load_module<Module>(std::get<Indices>(args_)...);
		}

	private:
		PackedArgs args_;
	};
}

template<typename Module, typename... Args>
auto load_module(Args&&... args) {
	using module_type = Module;
	using tuple_type = std::tuple<Args&&...>;
	using index_type = std::make_index_sequence<sizeof...(Args)>;

	return detail::load_module_package<
		module_type,
		tuple_type,
		index_type
	>(tuple_type{std::forward<Args>(args)...});
};

}

#endif //LIBSLIRC_LOAD_MODULE_HPP
