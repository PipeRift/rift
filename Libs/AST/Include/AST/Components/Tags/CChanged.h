// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	// Asigned to entities that have been modified during the last frame
	// Gets cleared after one frame
	struct CChanged : public p::Struct
	{
		STRUCT(CChanged, p::Struct, p::Struct_NotSerialized)
	};
}    // namespace rift::AST
