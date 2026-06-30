// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Tree.h>
#include <AST/Utils/TransactionUtils.h>
#include <Pipe/Core/StringView.h>
#include <PipeECS.h>


namespace rift::editor
{
	struct CTypeEditor;


	enum class DrawFieldFlags
	{
		None      = 0,
		HideValue = 1 << 0
	};

	using TVariableScopeRef = p::TIdScopeRef<p::Writes<ast::CDeclVariable, ast::CNamespace>,
	    ast::CDeclType, ast::CDeclNative, ast::CDeclStruct, ast::CDeclClass, ast::CParent>;

	void DrawField(ast::Tree& ast, CTypeEditor& editor, ast::Id functionId, ast::Id fieldId,
	    DrawFieldFlags flags = DrawFieldFlags::None);

	void DrawVariable(TVariableScopeRef scope, CTypeEditor& editor, ast::Id variableId);
	void DrawFunction(ast::Tree& ast, CTypeEditor& editor, ast::Id functionId);

	void DrawElementsPanel(ast::Tree& ast, ast::Id typeId);
}    // namespace rift::editor
