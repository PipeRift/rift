// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Tools/GraphPlayground.h"
#include "Uniques/CEditorUnique.h"
#include "Utils/FunctionGraph.h"

#include <UI/Nodes.h>

namespace Rift
{
	using namespace Nodes;

	void GraphPlayground::Draw(AST::Tree& ast, DockSpaceLayout& layout)
	{
		if (!open)
		{
			return;
		}

		layout.BindNextWindowToNode(CEditorUnique::centralNode);
		if (UI::Begin("Graph Playground", &open))
		{
			Graph::PushNodeStyle();
			Nodes::BeginNodeEditor();

			if (UI::IsWindowAppearing())
			{
				Graph::SetNodePosition(AST::Id(0), v2::Zero());
			}

			static bool boolValue = false;
			Graph::DrawBoolLiteralNode(AST::Id(0), boolValue);
			static String stringValue;
			Graph::DrawStringLiteralNode(AST::Id(1), stringValue);

			Graph::DrawCallNode(AST::Id(958), "Update");

			Nodes::MiniMap(0.2f, MiniMapLocation_TopRight);
			Nodes::EndNodeEditor();
			Graph::PopNodeStyle();
		}
		UI::End();
	}
}    // namespace Rift
