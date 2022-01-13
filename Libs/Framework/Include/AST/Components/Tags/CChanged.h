// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>


namespace Rift
{
	// Asigned to entities that have been modified during the last frame
	struct CChanged : public Struct
	{
		STRUCT(CChanged, Struct, Struct_NotSerialized)
	};
}    // namespace Rift
