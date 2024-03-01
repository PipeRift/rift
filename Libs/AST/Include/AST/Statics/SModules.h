// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::ast
{
	struct SModules : public p::Struct
	{
		P_STRUCT(SModules, p::Struct)

		p::TMap<p::Tag, ast::Id> modulesByPath;
	};
}    // namespace rift::ast
