/**********************************************************************
 *
 * variant.h
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

#ifndef _VARIANT_H
#define _VARIANT_H

#include "variant_common.h"

#include <iostream>			// std::cerr
#include <tuple>			// std::tuple
#include <string>			// std::string
#include <memory>			// std::unique_ptr
#include <typeinfo>			// typeid
#include <utility>			// std::index_sequence_for, std::index_sequence
#include <array>			// std::array
#include <algorithm>			// std::find
#include <typeindex>			// std::type_index

namespace
{
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
#if (defined(__GNUG__) && __cplusplus >= 201703) || (defined(_MSC_VER) && _MSC_VER >= 1914)
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

	template <typename T, typename U = std::decay_t<T>>
	bool LegalVariant(T&& arg)
	{
		if (std::is_same<char*, U>::value || std::is_same<const char*, U>::value)
			return true;
		return LegalVariantHelper(std::forward<T>(arg), var_tuple, std::index_sequence_for<VARIANTTYPES>{});
	}

	template <typename Tuple, typename Functor, size_t N>
	decltype(auto) visit_one(Tuple& tup, Functor func)
	{
		return func(std::get<N>(tup));
	}

	template <typename ReturnType, typename Tuple, typename Functor, size_t ... Is>
	decltype(auto) visit(Tuple& tup, size_t index, Functor func, std::index_sequence<Is...>)
	{
		using FT = ReturnType(*)(Tuple&, Functor);
		static constexpr std::array<FT, std::tuple_size<Tuple>::value> arr =
		{
			&visit_one<Tuple, Functor, Is>...
		};
		return arr[index](tup, func);
	}

	template <typename ReturnType, typename Functor, typename ... Args>
	decltype(auto) visit(std::tuple<Args...>& tup, size_t index, Functor func)
	{
		return visit<ReturnType>(tup, index, func, std::index_sequence_for<Args...>{});
	}
}

class Variant final
{
	struct AbstractImpl
	{
		virtual AbstractImpl& operator=(const AbstractImpl&) = default;
		virtual ~AbstractImpl() {}
	};

	template <typename T>
	struct VariantImpl : public AbstractImpl
	{
		T _data;

		VariantImpl(const T& value) : _data{ value } {}
		virtual ~VariantImpl() {}

	};

	std::unique_ptr<AbstractImpl> data;
	int type_id = -1;

public:
	Variant() = default;
	Variant(const Variant& rhs)
		: data{ }, type_id{ rhs.type_id }
	{
		auto func = [&rhs, this](auto&& elem)
		{
			using decayed = std::decay_t<decltype(elem)>;
			auto& rhs_elem = rhs.get_value<decayed>();
			data.reset( new VariantImpl<decayed>(rhs_elem) );
		};
		visit<void>(var_tuple, type_id, func);
	}

	Variant(Variant&& rhs)
		: data{ std::move(rhs.data) }, type_id{ rhs.type_id }
	{
		rhs.type_id = -1;
		rhs.data.reset();
	}

	template <typename T, typename U = std::decay_t<T>>
	Variant(const T& value)
		: data{ },
		  type_id{ }
	{
		// @edit: You could replace this section with your own error catching
		if (!LegalVariant(value))
		{
			std::cerr << "Unable to convert this type into a Variant!" << std::endl;
			throw;
		}

		constexpr bool char_star_check = std::is_same<char*, U>::value || std::is_same<const char*, U>::value;

#if (defined(__GNUG__) && __cplusplus >= 201703) || (defined(_MSC_VER) && _MSC_VER >= 1914)
		if constexpr (char_star_check)
		{
			type_id = GetSearchedIndex<std::string>(var_tuple, std::index_sequence_for<VARIANTTYPES>{});
			data.reset( new VariantImpl<std::string>(value) );
		}
		else
		{
			type_id = GetSearchedIndex<T>(var_tuple, std::index_sequence_for<VARIANTTYPES>{});
			data.reset( new VariantImpl<U>(value) );
		}
#else
		type_id = GetSearchedIndex<T>(var_tuple, std::index_sequence_for<VARIANTTYPES>{});
		data.reset( new VariantImpl<U>(value) );
#endif
	}

	Variant& operator=(const Variant& rhs)
	{
		type_id = rhs.type_id;
		auto func = [&rhs, this](auto&& elem)
		{
			using decayed = std::decay_t<decltype(elem)>;
			auto& rhs_elem = rhs.get_value<decayed>();
			data.reset( new VariantImpl<decayed>(rhs_elem) );
		};
		visit<void>(var_tuple, type_id, func);
		return *this;
	}

	Variant& operator=(Variant&& rhs)
	{
		std::swap(type_id, rhs.type_id);
		data.swap(rhs.data);
		return *this;
	}

	template <typename T>
	Variant& operator=(const T& value)
	{
		*this = std::move(Variant{ value });
		return *this;
	}

	template <typename T>
	T* get_pointer()
	{
		auto* ptr = dynamic_cast<VariantImpl<T>*>(data.get());
		return (!ptr) ? nullptr : &ptr->_data;
	}

	template <typename T>
	const T* get_pointer() const
	{
		const auto* ptr = dynamic_cast<const VariantImpl<T>*>(data.get());
		return (!ptr) ? nullptr : &ptr->_data;
	}

	template <typename T, typename U = std::decay_t<T>>
	U& get_value()
	{
		return dynamic_cast<VariantImpl<U>&>(*data.get())._data;
	}

	template <typename T, typename U = std::decay_t<T>>
	const U& get_value() const
	{
		return dynamic_cast<VariantImpl<U>&>(*data.get())._data;
	}

	template <typename T>
	void set_value(const T& value)
	{
		operator=(value);
	}

	bool has_value() const
	{
		return !*this;
	}

	unsigned index() const
	{
		return static_cast<unsigned>(type_id);
	}

	operator bool() const
	{
		return data.get() != nullptr && type_id != -1;
	}

	template <typename T, typename U = std::decay_t<T>>
	operator T() const
	{
		return get_value<U>();
	}

	template <typename T>
	bool operator==(const T& value) const
	{
		if (same_type(value))
			return get_value<T>() == value;
		return false;
	}

	template <typename T>
	bool operator!=(const T& value) const
	{
		return !operator==(value);
	}

	bool operator==(const Variant& rhs) const
	{
		auto func = [&rhs, this](auto&& elem) -> bool
		{
			using decayed = std::decay_t<decltype(elem)>;
			const auto* ptr_lhs = dynamic_cast<const VariantImpl<decayed>*>(data.get());
			const auto* ptr_rhs = dynamic_cast<const VariantImpl<decayed>*>(rhs.data.get());
			if (!ptr_lhs || !ptr_rhs)
				return false;
			return ptr_lhs->_data == ptr_rhs->_data;
		};
		return visit<bool>(var_tuple, type_id, func);
	}

	bool operator!=(const Variant& rhs) const
	{
		return !operator==(rhs);
	}

	template <typename T>
	bool same_type(const T& value) const
	{
		if (type_id == -1)
			return false;
		return GetTypeIndex(type_id, var_tuple, std::index_sequence_for<VARIANTTYPES>{}) == typeid(value);
	}

	std::type_index type() const
	{
		return GetTypeIndex(type_id, var_tuple, std::index_sequence_for<VARIANTTYPES>{});
	}
};

#endif
