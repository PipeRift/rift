// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>


namespace Rift
{
	// Marks a type as dirty, meaning is has been modified
	struct CTypeDirty : public Struct
	{
		STRUCT(CTypeDirty, Struct, Struct_NotSerialized)
	};
}    // namespace Rift
