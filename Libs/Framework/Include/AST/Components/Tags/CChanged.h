// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public Pipe::Struct
	{
		STRUCT(CChanged, Pipe::Struct, Pipe::Struct_NotSerialized)
	};
}    // namespace Rift
