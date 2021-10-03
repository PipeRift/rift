// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Tools/GraphPlayground.h"
#include "Uniques/CEditorUnique.h"
#include "Utils/FunctionGraph.h"

#include <imnodes.h>


void Rift::GraphPlayground::Draw(AST::Tree& ast, DockSpaceLayout& layout)
{
	if (!open)
	{
		return;
	}

	layout.BindNextWindowToNode(CEditorUnique::centralNode);
	if (UI::Begin("Graph Playground", &open))
	{
		Graph::PushNodeStyle();
		ImNodes::BeginNodeEditor();

		if (UI::IsWindowAppearing())
		{
			Graph::SetNodePosition(AST::Id(0), v3::Zero());
		}

		static bool boolValue = false;
		Graph::DrawBoolLiteralNode(AST::Id(0), boolValue);
		static String stringValue;
		Graph::DrawStringLiteralNode(AST::Id(1), stringValue);

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
		ImNodes::EndNodeEditor();
		Graph::PopNodeStyle();
	}
	UI::End();
}
