// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Types/Struct.h>


namespace Rift
{
	struct SModules : public Struct
	{
		STRUCT(SModules, Struct)

		TMap<Name, AST::Id> modulesByPath;
	};
}    // namespace Rift
