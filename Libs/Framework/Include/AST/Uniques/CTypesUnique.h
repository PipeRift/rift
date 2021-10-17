// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	struct CTypesUnique : public Struct
	{
		STRUCT(CTypesUnique, Struct)

		TMap<Name, AST::Id> typesByPath;
	};
}    // namespace Rift
