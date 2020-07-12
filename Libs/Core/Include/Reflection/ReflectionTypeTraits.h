// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Object/BaseObject.h"
#include "Strings/Name.h"

#include <EASTL/type_traits.h>

#include <type_traits>


namespace VCLang
{
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
	inline constexpr bool IsStructType()
	{
		return std::is_same_v<T, Struct> || std::is_convertible_v<T, Struct>;
	}

	template <typename T>
	inline constexpr bool IsObjectType()
	{
		return std::is_convertible_v<T, BaseObject>;
	}

	template <typename T>
	inline constexpr bool IsReflectableType()
	{
		if constexpr (IsArrayType<T>())
		{
			return IsReflectableType<typename T::ItemType>();
		}
		return /*IsAssetType<T>() || */ IsStructType<T>();
	}

	template <typename T>
	inline Name GetReflectableName()
	{
		if constexpr (IsArrayType<T>())
		{
			if constexpr (IsReflectableType<typename T::ItemType>())
			{
				// TArray<Itemtype> name
				return {CString::Format(TX("TArray<{}>"),
					GetReflectableName<typename T::ItemType>().ToString().c_str())};
			}
			return TX("TArray<Invalid>");
		}
		/*else if constexpr (IsAssetType<T>()) // TODO: Simplify container reflected names
		{
			// TAssetPtr<Itemtype> name
			return {CString::Format(TX("TAssetPtr<{}>"), GetReflectableName<typename
		T::ItemType>().ToString().c_str())};
		}*/
		else if constexpr (IsStructType<T>() || IsObjectType<T>())
		{
			return T::StaticType()->GetName();
		}
		return TX("Invalid");
	}
}	 // namespace VCLang


#define DECLARE_REFLECTION_TYPE(Type)               \
	template <>                                     \
	inline constexpr bool IsReflectableType<Type>() \
	{                                               \
		return true;                                \
	}                                               \
	template <>                                     \
	inline Name GetReflectableName<Type>()          \
	{                                               \
		static const Name typeName{TX(#Type)};      \
		return typeName;                            \
	}
