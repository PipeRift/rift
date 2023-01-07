// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	using namespace p::core;

	struct STypes : public Struct
	{
		STRUCT(STypes, Struct)

		TMap<Name, Id> typesByName;
		TMap<Name, Id> typesByPath;
	};
}    // namespace rift::AST
