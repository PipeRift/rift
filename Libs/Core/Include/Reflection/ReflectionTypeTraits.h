// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Strings/Name.h"

#include <type_traits>
#include <EASTL/type_traits.h>

struct Struct;


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

template <typename T>
inline constexpr bool IsArrayType()
{
	// Check if we are dealing with a TArray
	if constexpr (HasItemType<T>::value)
	{
		return std::is_same<TArray<typename T::ItemType>, T>::value;
	}
	return false;
}

template <typename T>
inline constexpr bool IsAssetType()
{
	// Check if we are dealing with a TAssetPtr
	if constexpr (HasItemType<T>::value)
	{
		return eastl::is_same<TAssetPtr<typename T::ItemType>, T>::value;
	}
	return false;
}

template <typename T>
inline constexpr bool IsStructType()
{
	return std::is_convertible<T, Struct>::value;
}

template <typename T>
inline constexpr bool IsReflectableType()
{
	if constexpr (IsArrayType<T>())
	{
		return IsReflectableType<typename T::ItemType>();
	}
	return IsAssetType<T>() || IsStructType<T>();
}

template <typename T>
inline Name GetReflectableName()
{
	if constexpr (IsArrayType<T>())
	{
		if constexpr (IsReflectableType<typename T::ItemType>())
		{
			// TArray<Itemtype> name
			return {CString::Format(TX("TArray<{}>"), GetReflectableName<typename T::ItemType>().ToString().c_str())};
		}
		return TX("TArray<Invalid>");
	}
	else if constexpr (IsAssetType<T>())
	{
		// TAssetPtr<Itemtype> name
		return {CString::Format(TX("TAssetPtr<{}>"), GetReflectableName<typename T::ItemType>().ToString().c_str())};
	}
	else if constexpr (IsStructType<T>())
	{
		return T::StaticStruct()->GetName();
	}

	return TX("Invalid");
}


#define DECLARE_REFLECTION_TYPE(Type)               \
	template <>                                     \
	inline constexpr bool IsReflectableType<Type>() \
	{                                               \
		return true;                                \
	}                                               \
	template <>                                     \
	inline Name GetReflectableName<Type>()          \
	{                                               \
		return TX(#Type);                           \
	}
