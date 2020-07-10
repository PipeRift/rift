// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Class.h"
#include "TProperty.h"


namespace Refl
{
	// Class will be specialized for each type at compile time and store
	// the metadata for that type.
	template <typename T>
	class TClass : public Class
	{
		static_assert(std::is_convertible<T, BaseObject>::value, "Type is not an Object!");

	private:
		static TClass _class;


	public:
		TClass() : Class()
		{
			T::__refl_Registry();
			T::__refl_RegistryProperties();
		}

	public:
		virtual GlobalPtr<BaseObject> CreateInstance(const Ptr<BaseObject>& owner) override
		{
			GlobalPtr<BaseObject> ptr = GlobalPtr<T>::Create(owner);
			return ptr;
		}

		static TClass* GetStatic()
		{
			return &_class;
		}
	};

	template <typename T>
	TClass<T> TClass<T>::_class{};
}	 // namespace Refl
