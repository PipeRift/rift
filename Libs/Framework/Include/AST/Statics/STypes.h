// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	struct STypes : public Struct
	{
		STRUCT(STypes, Struct)

		TMap<Name, AST::Id> typesByName;
		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace Rift
