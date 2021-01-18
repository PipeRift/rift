// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <stddef.h>

#include <type_traits>


/** SIZE SELECTORS */

namespace Rift
{
	template <class T, size_t size>
	struct IsSmallerType : std::integral_constant<bool, (sizeof(T) <= size)>
	{};

	template <class T, size_t size>
	struct IsBiggerType : std::integral_constant<bool, (sizeof(T) > size)>
	{};


#define EnableIfSmallerType(size) typename = std::enable_if<IsSmallerType<T, size>::value>
#define EnableIfNotSmallerType(size) typename = std::enable_if<!IsSmallerType<T, size>::value>

#define EnableIfBiggerType(size) typename = std::enable_if<IsBiggerType<T, size>::value>
#define EnableIfNotBiggerType(size) typename = std::enable_if<!IsBiggerType<T, size>::value>

#define EnableIfPassByValue(T)                                             \
	typename = std::enable_if < IsSmallerType<T, sizeof(size_t)>::value && \
			   std::is_copy_constructible<T>::type >
#define EnableIfNotPassByValue(T) \
	typename = std::enable_if<!(  \
		IsSmallerType<T, sizeof(size_t)>::value && std::is_copy_constructible<T>::type)>

#define EnableIfAll typename = void


	template <bool B, class T = void>
	using EnableIf = std::enable_if<B, T>;

	template <bool B, class T = void>
	using EnableIfT = std::enable_if_t<B, T>;


	template <typename T>
	struct HasItemType
	{
	private:
		template <typename V>
		static void impl(decltype(typename V::ItemType(), int()));
		template <typename V>
		static bool impl(char);

	public:
		static const bool value = std::is_void<decltype(impl<T>(0))>::value;
	};
}	 // namespace Rift
