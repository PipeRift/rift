// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct SModules : public p::Struct
	{
		STRUCT(SModules, p::Struct)

		p::TMap<p::Name, AST::Id> modulesByPath;
	};
}    // namespace rift::AST
