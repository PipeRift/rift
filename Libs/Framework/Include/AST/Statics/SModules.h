// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace Rift
{
	struct SModules : public Pipe::Struct
	{
		STRUCT(SModules, Pipe::Struct)

		Pipe::TMap<Pipe::Name, AST::Id> modulesByPath;
	};
}    // namespace Rift
