// Copyright 2015-2022 Piperift - All rights reserved

#include "Tools/GraphPlayground.h"

#include "DockSpaceLayout.h"
#include "Statics/SEditor.h"
#include "Utils/FunctionGraph.h"

#include <Utils/Nodes.h>
#include <Utils/NodesMiniMap.h>


namespace rift::Editor
{
	using namespace Nodes;

	void GraphPlayground::Draw(AST::Tree& ast, DockSpaceLayout& layout)
	{
		if (!open)
		{
			return;
		}

		layout.BindNextWindowToNode(SEditor::centralNode);
		if (UI::Begin("Graph Playground", &open))
		{
			Graph::PushNodeStyle();

			Nodes::SetEditorContext(&nodesEditor);
			Nodes::BeginNodeEditor();

			if (UI::IsWindowAppearing())
			{
				Graph::SetNodePosition(AST::Id(0), v2::Zero());
			}

			static bool boolValue = false;
			// Graph::Literals::DrawBoolNode(AST::Id(0), boolValue);
			static String stringValue;
			// Graph::Literals::DrawStringNode(AST::Id(1), stringValue);

			// Graph::DrawCallNode({}, AST::Id(958), "ApplyDamage", "DamageSystem");

			Nodes::DrawMiniMap(0.2f, MiniMapCorner::TopRight);
			Nodes::EndNodeEditor();
			Graph::PopNodeStyle();
		}
		UI::End();
	}
}    // namespace rift::Editor
