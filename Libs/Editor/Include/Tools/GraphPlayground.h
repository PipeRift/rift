// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift::editor
{
	struct GraphPlayground
	{
		bool open = false;

		Nodes::EditorContext nodesEditor;


		GraphPlayground() {}

		void Draw(ast::Tree& ast, struct DockSpaceLayout& layout);
	};
}    // namespace rift::editor
