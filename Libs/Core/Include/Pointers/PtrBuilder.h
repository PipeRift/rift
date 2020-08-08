// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Object/BaseObject.h"
#include "TypeTraits.h"


namespace VCLang
{
	template <typename T>
	struct PtrBuilder
	{
		template <typename... Args>
		static T* New(Args&&... args)
		{
			return new T(std::forward<Args>(args)...);
		}

		static T* NewArray(size_t size)
		{
			using Elem = std::remove_extent_t<T>;
			return new Elem[size]();
		}

		static void Delete(void* rawPtr)
		{
			T* ptr = static_cast<T*>(rawPtr);
			if constexpr (IsObject<T>::value)
			{
				ptr->StartDestroy();
			}
			delete ptr;
		}
	};
}	 // namespace VCLang
