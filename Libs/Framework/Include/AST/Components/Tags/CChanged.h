// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Types/Struct.h>


namespace Rift
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public Struct
	{
		STRUCT(CChanged, Struct, Struct_NotSerialized)
	};
}    // namespace Rift
