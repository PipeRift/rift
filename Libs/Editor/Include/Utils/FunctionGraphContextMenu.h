// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <AST/Tree.h>
#include <PipeECS.h>


namespace rift::editor
{
	struct MenuContext;
}

namespace rift::editor::Graph
{
	void DrawNodesContextMenu(ast::Tree& ast, ast::Id typeId, p::TView<ast::Id> nodeIds);
	void DrawGraphContextMenu(ast::Tree& ast, ast::Id typeId, MenuContext context);
	void DrawContextMenu(ast::Tree& ast, ast::Id typeId, MenuContext context);
}    // namespace rift::editor::Graph
