// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "BaseStruct.h"

#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/TStruct.h"
#include "Core/Serialization/Archive.h"


struct Struct : public BaseStruct {
	ORPHAN_STRUCT(Struct)

	virtual StructType* GetStruct() const { return StaticStruct(); }
};
