// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <PRefl/Struct.h>


namespace rift
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public p::Struct
	{
		STRUCT(CChanged, p::Struct, p::Struct_NotSerialized)
	};
}    // namespace rift
