// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	struct SModules : public Struct
	{
		STRUCT(SModules, Struct)

		TMap<Name, AST::Id> modulesByPath;
	};
}    // namespace Rift
