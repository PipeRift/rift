// Copyright 2015-2020 Piperift - All rights reserved

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/FunctionGraph.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Linkage.h>
#include <AST/Uniques/CTypeListUnique.h>
#include <imnodes.h>
#include <imnodes_internal.h>


namespace Rift
{
	void DrawFunctionNodes(AST::Tree& ast, AST::Id functionId);

	void DrawFunctionGraph(AST::Tree& ast, AST::Id typeId, DockSpaceLayout& layout)
	{
		TArray<AST::Id>* children = AST::GetLinked(ast, typeId);
		if (!children)
		{
			return;
		}

		layout.BindNextWindowToNode(CTypeEditor::centralNode);
		static String graphId;
		graphId.clear();
		Strings::FormatTo(graphId, "Graph##{}", typeId);
		UI::Begin(graphId.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

		ImNodes::GetStyle().Flags |=
		    ImNodesStyleFlags_GridLines /* | ImNodesStyleFlags_GridSnappingOnRelease*/;
		ImNodes::PushStyleVar(ImNodesStyleVar_GridSpacing, 12.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinLineThickness, 2.5f);
		ImNodes::PushStyleVar(ImNodesStyleVar_NodeCornerRounding, 2.f);
		ImNodes::PushStyleVar(ImNodesStyleVar_PinQuadSideLength, 10.f);

		ImNodes::BeginNodeEditor();

		if (!ImGui::IsAnyItemHovered() && ImNodes::IsEditorHovered() &&
		    ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("GraphContextMenu");
		}

		if (ImGui::BeginPopup("GraphContextMenu"))
		{
			static ImGuiTextFilter filter;
			filter.Draw("##Filter");
			const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
			if (filter.IsActive() || ImGui::TreeNode("Constructors"))
			{
				String makeStr{};
				auto& typeList   = ast.GetUnique<CTypeListUnique>();
				auto identifiers = ast.MakeView<CIdentifier>();
				for (auto type : typeList.types)
				{
					if (CIdentifier* iden = identifiers.TryGet<CIdentifier>(type.second))
					{
						makeStr.clear();
						Strings::FormatTo(makeStr, "Make {}", iden->name);
						if (filter.PassFilter(makeStr.c_str(), makeStr.c_str() + makeStr.size()))
						{
							if (ImGui::MenuItem(makeStr.c_str())) {}
						}
					}
				}

				if (!filter.IsActive())
				{
					ImGui::TreePop();
				}
			}
			ImGui::EndPopup();
		}


		auto functions = ast.MakeView<CFunctionDecl>();
		for (AST::Id child : *children)
		{
			if (functions.Has(child))
			{
				DrawFunctionNodes(ast, child);
			}
		}

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
		ImNodes::EndNodeEditor();

		UI::End();
	}

	void DrawFunctionEntry(AST::Tree& ast, AST::Id functionId)
	{
		auto* imnodes = ImNodes::GetCurrentContext();

		Name name;
		if (auto* identifier = ast.TryGet<CIdentifier>(functionId))
		{
			name = identifier->name;
		}

		auto& transform = ast.GetOrAdd<CGraphTransform>(functionId);
		if (UI::IsWindowAppearing() &&
		    !(imnodes->LeftMouseDragging && ImNodes::IsNodeSelected(i32(functionId))))
		{
			ImNodes::SetNodeGridSpacePos(i32(functionId), transform.position);
		}

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(191, 56, 11, 255));
		ImNodes::BeginNode(i32(functionId));

		ImNodes::BeginNodeTitleBar();
		UI::Text(name.ToString().c_str());
		ImNodes::EndNodeTitleBar();

		UI::BeginGroup();    // Outputs
		{
			ImNodes::BeginOutputAttribute(i32(functionId), ImNodesPinShape_QuadFilled);
			ImNodes::EndOutputAttribute();
		}
		UI::EndGroup();

		ImNodes::EndNode();
		ImNodes::PopColorStyle();

		if (imnodes->LeftMouseDragging || imnodes->LeftMouseReleased)
		{
			ImVec2 pos           = ImNodes::GetNodeGridSpacePos(i32(functionId));
			transform.position.x = pos.x;
			transform.position.y = pos.y;
			Types::Changed(AST::GetLinkedParent(ast, functionId), "Moved nodes");
		}
	}

	void DrawFunctionNodes(AST::Tree& ast, AST::Id functionId)
	{
		DrawFunctionEntry(ast, functionId);
	}
}    // namespace Rift
