// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace rift
{
	struct SModules : public pipe::Struct
	{
		STRUCT(SModules, pipe::Struct)

		pipe::TMap<pipe::Name, AST::Id> modulesByPath;
	};
}    // namespace rift
