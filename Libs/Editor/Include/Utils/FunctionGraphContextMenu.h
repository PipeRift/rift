// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Pipe/PipeECS.h>


namespace rift::Editor::Graph
{
	void DrawNodesContextMenu(AST::Tree& ast, AST::Id typeId, p::TView<AST::Id> nodeIds);
	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId);
	void DrawContextMenu(
	    AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId, AST::Id hoveredLinkId);
}    // namespace rift::Editor::Graph
