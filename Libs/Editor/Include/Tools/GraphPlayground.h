// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift::Editor
{
	struct GraphPlayground
	{
		bool open = false;

		Nodes::EditorContext nodesEditor;


		GraphPlayground() {}

		void Draw(AST::Tree& ast, struct DockSpaceLayout& layout);
	};
}    // namespace rift::Editor
