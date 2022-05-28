// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Filtering.h>
#include <AST/Tree.h>


namespace Rift::Graph
{
	void DrawNodesContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> nodeIds);
	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId);
	void DrawContextMenu(
	    AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId, AST::Id hoveredLinkId);
}    // namespace Rift::Graph
