// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <PipeReflect.h>


namespace rift::ast
{
	struct SModules
	{
		P_STRUCT(SModules)

		p::TMap<p::Tag, ast::Id> modulesByPath;
	};
}    // namespace rift::ast
