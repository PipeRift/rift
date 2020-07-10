// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "CoreEngine.h"
#include "Reflection/Reflection.h"
#include "Reflection/Struct.h"


struct Struct : public BaseStruct
{
	ORPHAN_STRUCT(Struct, ReflectionTags::None)

	virtual Refl::Struct* GetType() const
	{
		return StaticType();
	}
};
