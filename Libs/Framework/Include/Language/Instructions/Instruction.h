// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"

#include <Object/Struct.h>


namespace Rift
{
	struct Instruction : public Struct
	{
		STRUCT(Instruction, Struct)

		PROP(v2, position)
		v2 position{};
	};
}    // namespace Rift
