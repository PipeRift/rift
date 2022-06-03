// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Types/Struct.h>


namespace Rift
{
	struct STypes : public Struct
	{
		STRUCT(STypes, Struct)

		TMap<Name, AST::Id> typesByName;
		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace Rift
