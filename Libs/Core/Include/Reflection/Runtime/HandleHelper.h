// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Object/BaseObject.h"
#include "Object/ObjectPtr.h"
#include "Reflection/Runtime/ClassHandle.h"


class HandleHelper
{
public:
	// #TODO: Move to TClass
	static eastl::shared_ptr<ClassHandle> CreateClassHandle(const Ptr<BaseObject>& instance)
	{
		return eastl::make_shared<ClassHandle>(instance);
	}
};
