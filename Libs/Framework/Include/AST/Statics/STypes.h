// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace Rift
{
	struct STypes : public Pipe::Struct
	{
		STRUCT(STypes, Pipe::Struct)

		TMap<Name, AST::Id> typesByName;
		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace Rift
