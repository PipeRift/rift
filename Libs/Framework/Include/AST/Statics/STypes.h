// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace rift
{
	struct STypes : public pipe::Struct
	{
		STRUCT(STypes, pipe::Struct)

		TMap<Name, AST::Id> typesByName;
		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace rift
