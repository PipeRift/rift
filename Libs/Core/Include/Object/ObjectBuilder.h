// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "BaseObject.h"
#include "Log.h"
#include "Reflection/Class.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder
	{
		static PtrOwner<T, ObjectBuilder> New(Refl::Class* objectClass, Ptr<BaseObject> owner = {})
		{
			static_assert(IsObject<T>::value, "Type is not an Object!");
			if (objectClass)
			{
				return objectClass->CreateInstance(owner).Cast<T>();
			}
			return {};
		}

		static PtrOwner<T, ObjectBuilder> New(Ptr<BaseObject> owner = {})
		{
			static_assert(IsObject<T>::value, "Type is not an Object!");

			if constexpr (std::is_abstract_v<T>)
			{
				Log::Error("Tried to create an instance of '{}' which is abstract.",
					GetReflectableName<T>().ToString());
				return {};
			}
			else
			{
				PtrOwner<T, ObjectBuilder> ptr{new T()};
				ptr->PreConstruct(ptr.AsPtr(), T::StaticType(), owner);
				ptr->Construct();
				return ptr;
			}
		}

		static void Delete(void* rawPtr)
		{
			T* ptr = static_cast<T*>(rawPtr);
			ptr->StartDestroy();
			delete ptr;
		}
	};
}	 // namespace Rift