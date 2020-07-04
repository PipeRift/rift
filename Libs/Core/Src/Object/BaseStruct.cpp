// Copyright 2015-2020 Piperift - All rights reserved

#include "Object/BaseStruct.h"

#include "Object/Struct.h"
#include "Reflection/StructType.h"



StructType* BaseStruct::GetStruct() const
{
	if (const auto* thisObj = dynamic_cast<const Struct*>(this))
	{
		return thisObj->GetStruct();
	}
	return nullptr;
}
