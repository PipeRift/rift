// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <PipeECS.h>


namespace rift::editor::Graph
{
	void DrawNodesContextMenu(ast::Tree& ast, ast::Id typeId, p::TView<ast::Id> nodeIds);
	void DrawGraphContextMenu(ast::Tree& ast, ast::Id typeId, ast::Id hoveredNodeId);
	void DrawContextMenu(
	    ast::Tree& ast, ast::Id typeId, ast::Id hoveredNodeId, ast::Id hoveredLinkId);
}    // namespace rift::editor::Graph
