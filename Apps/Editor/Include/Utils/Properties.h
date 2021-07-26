// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"


namespace Rift::Editor
{
	void DrawVariable(AST::Tree& ast, AST::Id variableId);
	void DrawFunction(AST::Tree& ast, AST::Id functionId);

	void DrawVariables(AST::Tree& ast, AST::Id typeId);
	void DrawFunctions(AST::Tree& ast, AST::Id typeId);

	void DrawProperties(AST::Tree& ast, AST::Id typeId, struct DockSpaceLayout& layout);
}    // namespace Rift::Editor
