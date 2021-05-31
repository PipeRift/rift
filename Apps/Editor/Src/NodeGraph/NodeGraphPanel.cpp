// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Editors/TypeAssetEditor.h"
#include "NodeGraph/NodeGraphPanel.h"
#include "UI/UI.h"

#include <imnodes.h>



namespace Rift
{
	NodeGraphPanel::NodeGraphPanel(TypeAssetEditor& editor) : editor(editor) {}

	void NodeGraphPanel::Draw(DockSpaceLayout& layout)
	{
		ImNodes::GetStyle().Flags |=
		    ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnappingOnRelease;
		ImNodes::PushStyleVar(ImNodesStyleVar_GridSpacing, 12.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinLineThickness, 2.5f);
		ImNodes::PushStyleVar(ImNodesStyleVar_NodeCornerRounding, 2.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinQuadSideLength, 10.f);

		layout.BindNextWindowToNode(TypeAssetEditor::centralNode);

		const String name = Strings::Format(TX("Graph##{}"), editor.GetWindowId());
		if (ImGui::Begin(name.c_str()))
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
				ImNodes::PopColorStyle();
			}

			{    // Node 2
				ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(191, 56, 11, 255));
				ImNodes::BeginNode(2);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text("If");
				ImNodes::EndNodeTitleBar();

				ImGui::BeginGroup();    // Inputs
				{
					ImNodes::BeginInputAttribute(2, ImNodesPinShape_QuadFilled);
					ImGui::Text("");
					ImNodes::EndInputAttribute();

					ImNodes::BeginInputAttribute(3, ImNodesPinShape_CircleFilled);
					ImGui::Text("Value");
					ImNodes::EndInputAttribute();
				}
				ImGui::EndGroup();

				ImGui::SameLine();
				ImGui::BeginGroup();    // Outputs
				{
					ImNodes::BeginOutputAttribute(4, ImNodesPinShape_QuadFilled);
					ImGui::Text("True");
					ImNodes::EndOutputAttribute();

					ImNodes::BeginOutputAttribute(5, ImNodesPinShape_QuadFilled);
					ImGui::Text("False");
					ImNodes::EndOutputAttribute();
				}
				ImGui::EndGroup();

				ImNodes::EndNode();
				ImNodes::PopColorStyle();
			}
			ImNodes::EndNodeEditor();
		}
		ImGui::End();
	}
}    // namespace Rift
