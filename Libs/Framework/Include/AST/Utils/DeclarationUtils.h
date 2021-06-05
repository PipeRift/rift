// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Components/CClassDecl.h"
#include "AST/Tree.h"


namespace Rift::Util
{
	bool IsStructDecl(const AST::Tree& ast, AST::Id node)
	{
		return ast.Has<CClassDecl>(node);
	}
}    // namespace Rift::Util
