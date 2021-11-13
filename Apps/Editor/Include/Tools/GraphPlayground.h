// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/NodesInternal.h>
#include <UI/UI.h>


namespace Rift
{
	struct GraphPlayground
	{
		bool open = true;

		Nodes::EditorContext nodesEditor;


		GraphPlayground() {}

		void Draw(AST::Tree& ast, struct DockSpaceLayout& layout);
	};
}    // namespace Rift
