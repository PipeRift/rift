// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Tree.h"


namespace Rift::Declarations
{
	bool IsClass(const AST::Tree& ast, AST::Id node)
	{
		return ast.Has<CClassDecl>(node);
	}

	bool IsStruct(const AST::Tree& ast, AST::Id node)
	{
		return ast.Has<CStructDecl>(node);
	}
}    // namespace Rift::Util
