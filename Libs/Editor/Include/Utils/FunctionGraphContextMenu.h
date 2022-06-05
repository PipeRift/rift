// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <ECS/Id.h>


namespace rift::Graph
{
	void DrawNodesContextMenu(AST::Tree& ast, AST::Id typeId, pipe::TSpan<AST::Id> nodeIds);
	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId);
	void DrawContextMenu(
	    AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId, AST::Id hoveredLinkId);
}    // namespace rift::Graph
