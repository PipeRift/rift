// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <EASTL/vector.h>

#include "BaseType.h"
#include "Core/Object/BaseObject.h"
#include "Core/Object/ObjectPtr.h"


class Class : public BaseType
{
public:

	virtual GlobalPtr<BaseObject> CreateInstance(const Ptr<BaseObject>& owner) = 0;


	// NOTE: Most of the class comparison functions do actually
	// call BaseType to reduce complexity and code duplication.
	//
	// We can cast safely to BaseType since Classes only inherit Classes

	Class* GetParent() const { return static_cast<Class*>(parent); }

	void GetAllChildren(TArray<Class*>& outChildren) {
		__GetAllChildren(reinterpret_cast<TArray<BaseType*>&>(outChildren));
	}

	Class* FindChild(Name className) const {
		return static_cast<Class*>(__FindChild(className));
	}

	template<typename Type>
	bool IsChildOf() const { return BaseType::IsChildOf(Type::StaticClass()); }

	bool IsA(Class* other) const { return this == other; }
};
