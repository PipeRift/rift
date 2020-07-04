// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "CoreEngine.h"
#include "Reflection/Reflection.h"
#include "Reflection/TStruct.h"
#include "Serialization/Archive.h"



struct Struct : public BaseStruct
{
	ORPHAN_STRUCT(Struct)

	virtual StructType* GetStruct() const
	{
		return StaticStruct();
	}
};
