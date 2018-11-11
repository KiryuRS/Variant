/**********************************************************************
 *
 * variant_helper.h
 *
 *
 *
 * LICENSE (http://www.opensource.org/licenses/bsd-license.php)
 *
 *   Copyright (c) 2018, KiryuRS
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without modification,
 *   are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *   Neither the name of Jochen Kalmbach nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * **********************************************************************/
// GNU Version
#ifdef __GNUG__
#if __cplusplus >= 201703
#define C17
#elif __cplusplus >= 201402
#define C14
#endif
#endif

// MS Version
#ifdef _MSC_VER
#if _MSC_VER >= 1914
#define C17
#elif _MSC_VER >= 1900
#define C14
#endif
#endif

#include <tuple>			// std::tuple
#include <memory>			// std::unique_ptr
#include <typeinfo>			// typeid
#include <utility>			// std::index_sequence_for, std::index_sequence
#include <array>			// std::array
#include <algorithm>		// std::find
#include <typeindex>		// std::type_index
#include <type_traits>		// std::common_type, std::invoke_result, std::result_of

std::tuple<VARIANTTYPES> var_tuple;

template <typename Search, typename ... Args, size_t ... N>
int GetSearchedIndex(const std::tuple<Args...>& tup, std::index_sequence<N...>)
{
	std::array<bool, sizeof...(Args)> arr
	{
		( typeid(Search) == typeid(std::get<N>(tup)) ) ...
	};
	auto iter = std::find(arr.begin(), arr.end(), true);
	if (iter != arr.end())
		return iter - arr.begin();
	return -1;
}

template <typename ... Args, size_t ... N>
std::type_index GetTypeIndex(int index, const std::tuple<Args...>& tup, std::index_sequence<N...>)
{
	std::array<std::type_index, sizeof...(Args)> arr
	{
		( typeid(std::get<N>(tup)) )...
	};

	return arr[index];
}

template <typename T, typename ... Args, size_t ... Is>
bool LegalVariantHelper(T&& arg, const std::tuple<Args...>& tup, std::index_sequence<Is...>)
{
#ifdef C17
	return [](const auto& a0, const auto& ... rest)
			{
				return ((typeid(a0) == typeid(rest)) || ...);
			}(arg, std::get<Is>(tup)...);
#else
	std::array<bool, sizeof...(Args)> arr
	{
		( typeid(arg) == typeid(std::get<Is>(tup)) ) ...
	};
	auto iter = std::find(arr.begin(), arr.end(), true);
	return iter != arr.end();
#endif
}

template <typename Tuple, typename Functor, size_t N>
#ifdef C17
std::invoke_result_t<Functor, decltype(std::get<N>(std::declval<Tuple>()))>
#else
std::result_of_t<Functor(decltype(std::get<N>(std::declval<Tuple>())))>
#endif
visit_one(Tuple& tup, Functor func)
{
	return func(std::get<N>(tup));
}

template <typename Tuple, typename Functor, size_t ... Is>
decltype(auto) visit_helper(Tuple& tup, size_t index, Functor func, std::index_sequence<Is...>)
{
	using type = std::common_type_t<decltype(visit_one<Tuple, Functor, Is>(tup, func))...>;
	using FT = type(*)(Tuple&, Functor);
	static constexpr std::array<FT, std::tuple_size<Tuple>::value> arr =
	{
		&visit_one<Tuple, Functor, Is>...
	};
	return arr[index](tup, func);
}