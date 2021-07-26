// Copyright 2015-2020 Piperift - All rights reserved

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/FunctionGraph.h"

#include <imnodes.h>


namespace Rift::Editor
{
	void DrawFunctionGraph(AST::Tree& ast, AST::Id functionId, DockSpaceLayout& layout)
	{
		ImNodes::GetStyle().Flags |=
		    ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnappingOnRelease;
		ImNodes::PushStyleVar(ImNodesStyleVar_GridSpacing, 12.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinLineThickness, 2.5f);
		ImNodes::PushStyleVar(ImNodesStyleVar_NodeCornerRounding, 2.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinQuadSideLength, 10.f);

		layout.BindNextWindowToNode(CTypeEditor::centralNode);

		if (UI::Begin("Graph"))
		{
			ImNodes::BeginNodeEditor();

			{    // Node
				ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
				ImNodes::BeginNode(1);

				ImNodes::BeginNodeTitleBar();
				UI::Text("If");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(2, ImNodesPinShape_QuadFilled);
				UI::Text("");
				ImNodes::EndInputAttribute();

				UI::SameLine();

				ImNodes::BeginOutputAttribute(4, ImNodesPinShape_QuadFilled);
				UI::Text("True");
				ImNodes::EndOutputAttribute();

				ImNodes::BeginInputAttribute(3, ImNodesPinShape_CircleFilled);
				UI::Text("Value");
				ImNodes::EndInputAttribute();

				UI::SameLine();

				ImNodes::BeginOutputAttribute(5, ImNodesPinShape_QuadFilled);
				UI::Text("False");
				ImNodes::EndOutputAttribute();

				ImNodes::EndNode();
				ImNodes::PopColorStyle();
			}

			{    // Node 2
				ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(191, 56, 11, 255));
				ImNodes::BeginNode(2);

				ImNodes::BeginNodeTitleBar();
				UI::Text("If");
				ImNodes::EndNodeTitleBar();

				UI::BeginGroup();    // Inputs
				{
					ImNodes::BeginInputAttribute(2, ImNodesPinShape_QuadFilled);
					UI::Text("");
					ImNodes::EndInputAttribute();

					ImNodes::BeginInputAttribute(3, ImNodesPinShape_CircleFilled);
					UI::Text("Value");
					ImNodes::EndInputAttribute();
				}
				UI::EndGroup();

				UI::SameLine();
				UI::BeginGroup();    // Outputs
				{
					ImNodes::BeginOutputAttribute(4, ImNodesPinShape_QuadFilled);
					UI::Text("True");
					ImNodes::EndOutputAttribute();

					ImNodes::BeginOutputAttribute(5, ImNodesPinShape_QuadFilled);
					UI::Text("False");
					ImNodes::EndOutputAttribute();
				}
				UI::EndGroup();

				ImNodes::EndNode();
				ImNodes::PopColorStyle();
			}
			ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
			ImNodes::EndNodeEditor();
		}
		UI::End();
	}
}    // namespace Rift::Editor
