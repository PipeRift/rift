// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Object/BaseObject.h"
#include "Reflection/Runtime/ClassHandle.h"


namespace Rift::Refl
{
	class HandleHelper
	{
	public:
		// #TODO: Move to TClass
		static std::shared_ptr<ClassHandle> CreateClassHandle(const Ptr<BaseObject>& instance)
		{
			return std::make_shared<ClassHandle>(instance);
		}
	};
}	 // namespace Rift::Refl
