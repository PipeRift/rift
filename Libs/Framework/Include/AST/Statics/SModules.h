// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace rift
{
	struct SModules : public p::Struct
	{
		STRUCT(SModules, p::Struct)

		p::TMap<p::Name, AST::Id> modulesByPath;
	};
}    // namespace rift
