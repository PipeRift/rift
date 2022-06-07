// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct STypes : public p::Struct
	{
		STRUCT(STypes, p::Struct)

		TMap<Name, AST::Id> typesByName;
		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace rift
