// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Reflection/Struct.h"
#include "TProperty.h"

#include <type_traits>


namespace Refl
{
	/** TStruct will be specialized for each type at compile time and store
	 * the metadata for that type
	 */
	template <typename T>
	class TStruct : public Struct
	{
		static_assert(std::is_convertible<T, ::Struct>::value, "Type does not inherit Struct!");

	private:
		static TStruct _struct;


	public:
		TStruct() : Struct()
		{
			T::__refl_Registry();
			T::__refl_RegistryProperties();
		}

		virtual BaseStruct* New() const override
		{
			return new T();
		}

		static TStruct* GetStatic()
		{
			return &_struct;
		}
	};

	template <typename T>
	TStruct<T> TStruct<T>::_struct{};
}	 // namespace Refl
