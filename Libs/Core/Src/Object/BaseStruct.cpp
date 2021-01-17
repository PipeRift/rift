// Copyright 2015-2020 Piperift - All rights reserved

#include "Object/BaseStruct.h"

#include "Object/Struct.h"
#include "Reflection/Struct.h"


namespace Rift
{
	Refl::Struct* BaseStruct::GetType() const
	{
		if (const auto* thisObj = dynamic_cast<const Struct*>(this))
		{
			return thisObj->GetType();
		}
		return nullptr;
	}
}	 // namespace Rift
