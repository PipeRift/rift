// Copyright 2015-2020 Piperift - All rights reserved

#include "Object/BaseObject.h"

#include "Object/Object.h"
#include "Reflection/Reflection.h"
#include "Reflection/TClass.h"


namespace VCLang
{
	Refl::Class* BaseObject::GetType() const
	{
		const auto* thisObj = static_cast<const Object*>(this);
		return thisObj ? thisObj->GetType() : nullptr;
	}

	Ptr<Object> BaseObject::Self() const
	{
		const auto* thisObj = static_cast<const Object*>(this);
		return thisObj ? thisObj->Self() : Ptr<Object>{};
	}
}	 // namespace VCLang
