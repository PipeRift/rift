// Copyright 2015-2021 Piperift - All rights reserved

#include "NodeGraph/NodeGraphPanel.h"

#include "DockSpaceLayout.h"
#include "Editors/AssetEditor.h"
#include "UI/UI.h"

#include <imnodes.h>


namespace Rift
{
	NodeGraphPanel::NodeGraphPanel() {}
	NodeGraphPanel::~NodeGraphPanel() {}

	void NodeGraphPanel::Draw(DockSpaceLayout& layout)
	{
		ImNodes::PushStyleVar(ImNodesStyleVar_PinLineThickness, 2.5f);
		ImNodes::PushStyleVar(ImNodesStyleVar_GridSpacing, 8.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_NodeCornerRounding, 2.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinQuadSideLength, 10.f);

		layout.BindNextWindowToNode(AssetEditor::centralNode);
		if (ImGui::Begin("Graph"))
		{
			ImNodes::BeginNodeEditor();

			{    // Node
				ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
				ImNodes::BeginNode(1);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text("If");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(2, ImNodesPinShape_QuadFilled);
				ImGui::Text("");
				ImNodes::EndInputAttribute();

				ImGui::SameLine();

				ImNodes::BeginOutputAttribute(4, ImNodesPinShape_QuadFilled);
				ImGui::Text("True");
				ImNodes::EndOutputAttribute();

				ImNodes::BeginInputAttribute(3, ImNodesPinShape_CircleFilled);
				ImGui::Text("Value");
				ImNodes::EndInputAttribute();

				ImGui::SameLine();

				ImNodes::BeginOutputAttribute(5, ImNodesPinShape_QuadFilled);
				ImGui::Text("False");
				ImNodes::EndOutputAttribute();

				ImNodes::EndNode();
			}
			ImNodes::EndNodeEditor();
		}
		ImGui::End();
	}
}    // namespace Rift
