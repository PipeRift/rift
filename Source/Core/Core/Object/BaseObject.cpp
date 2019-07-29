// Copyright 2015-2019 Piperift - All rights reserved

#include "BaseObject.h"
#include "Core/Reflection/TClass.h"
#include "Object.h"

Class* BaseObject::GetClass() const
{
	const auto* thisObj = static_cast<const Object*>(this);
	if (thisObj)
		return thisObj->GetClass();
	return nullptr;
}

Ptr<Object> BaseObject::Self() const
{
	const auto* thisObj = static_cast<const Object*>(this);
	if (thisObj)
		return thisObj->Self();
	return {};
}
