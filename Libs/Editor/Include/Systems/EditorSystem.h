// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <AST/Tree.h>


namespace rift::editor::EditorSystem
{
	void Init(ast::Tree& ast);
	void Draw(ast::Tree& ast);
	void OnTypeEditorOpen(ast::Tree& ast, ast::Id typeId);
}    // namespace rift::editor::EditorSystem
