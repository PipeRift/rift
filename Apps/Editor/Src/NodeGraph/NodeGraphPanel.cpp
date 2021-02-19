// Copyright 2015-2021 Piperift - All rights reserved

#include "NodeGraph/NodeGraphPanel.h"

#include "DockSpaceLayout.h"
#include "Editors/AssetEditor.h"

#include <imgui.h>
#include <imnodes.h>


NodeGraphPanel::NodeGraphPanel() {}
NodeGraphPanel::~NodeGraphPanel() {}

void NodeGraphPanel::Draw(DockSpaceLayout& layout)
{
	imnodes::PushStyleVar(imnodes::StyleVar_PinLineThickness, 2.5f);
	imnodes::PushStyleVar(imnodes::StyleVar_GridSpacing, 8.f);

	layout.BindNextWindowToNode(AssetEditor::centralNode);
	if (ImGui::Begin("Graph"))
	{
		imnodes::BeginNodeEditor();

		{    // Node
			imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(11, 109, 191, 255));
			imnodes::BeginNode(1);

			imnodes::BeginNodeTitleBar();
			ImGui::Text("If");
			imnodes::EndNodeTitleBar();

			imnodes::BeginInputAttribute(2, imnodes::PinShape_Triangle);
			ImGui::Text("");
			imnodes::EndInputAttribute();

			ImGui::SameLine();

			imnodes::BeginOutputAttribute(4, imnodes::PinShape_Triangle);
			ImGui::Text("True");
			imnodes::EndOutputAttribute();

			imnodes::BeginInputAttribute(3, imnodes::PinShape_CircleFilled);
			ImGui::Text("Value");
			imnodes::EndInputAttribute();

			ImGui::SameLine();

			imnodes::BeginOutputAttribute(5, imnodes::PinShape_Triangle);
			ImGui::Text("False");
			imnodes::EndOutputAttribute();

			imnodes::EndNode();
		}
		imnodes::EndNodeEditor();
	}
	ImGui::End();
}
