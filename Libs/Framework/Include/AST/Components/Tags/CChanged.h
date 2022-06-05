// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public pipe::Struct
	{
		STRUCT(CChanged, pipe::Struct, pipe::Struct_NotSerialized)
	};
}    // namespace rift
