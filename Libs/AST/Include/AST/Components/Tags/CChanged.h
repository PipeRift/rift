// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::ast
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public p::Struct
	{
		P_STRUCT(CChanged, p::Struct, p::Struct_NotSerialized)
	};
}    // namespace rift::ast
