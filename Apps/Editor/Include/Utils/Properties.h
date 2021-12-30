// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Strings/StringView.h>


namespace Rift
{
	struct CTypeEditor;


	enum class DrawFieldFlags
	{
		None      = 0,
		HideValue = 1 << 0
	};
	void DrawField(AST::Tree& ast, CTypeEditor& editor, AST::Id fieldId,
	    DrawFieldFlags flags = DrawFieldFlags::None);

	void DrawVariable(AST::Tree& ast, CTypeEditor& editor, AST::Id variableId);
	void DrawFunction(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId);

	void DrawVariables(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId);
	void DrawFunctions(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId);

	void DrawProperties(AST::Tree& ast, AST::Id typeId);
}    // namespace Rift
