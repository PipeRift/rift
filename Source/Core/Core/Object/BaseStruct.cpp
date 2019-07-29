// Copyright 2015-2019 Piperift - All rights reserved

#include "BaseStruct.h"
#include "Core/Reflection/StructType.h"
#include "Struct.h"


StructType* BaseStruct::GetStruct() const
{
	const Struct* thisObj = dynamic_cast<const Struct*>(this);
	if (thisObj)
		return thisObj->GetStruct();
	return nullptr;
}
