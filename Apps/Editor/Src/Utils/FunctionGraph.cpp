// Copyright 2015-2020 Piperift - All rights reserved

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/FunctionGraph.h"
#include "Utils/GraphColors.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Linkage.h>
#include <AST/Uniques/CTypeListUnique.h>
#include <UI/Nodes.h>
#include <UI/NodesInternal.h>
#include <UI/Style.h>


namespace Rift::Graph
{
	using namespace Nodes;

	void Settings::SetGridSize(float size)
	{
		gridSize                        = size;
		invGridSize                     = 1.f / size;
		Nodes::GetStyle().GridSpacing   = size;
	}

	float Settings::GetGridSize() const
	{
		return gridSize;
	}

	float Settings::GetInvGridSize() const
	{
		return invGridSize;
	}

	float Settings::GetSpaceHeight(u32 height) const
	{
		return settings.GetGridSize() - (settings.verticalMargin * 2.f);
	}

	v2 Settings::GetContentPadding() const
	{
		return {0.f, settings.verticalMargin + settings.verticalPadding};
	}


	void Graph::Init()
	{
		Nodes::CreateContext();
		settings.SetGridSize(settings.GetGridSize());
	}

	void Graph::Shutdown()
	{
		Nodes::DestroyContext();
	}


	void PushNodeStyle()
	{
		Nodes::GetStyle().Flags |=
		    StyleFlags_GridLines | StyleFlags_GridLinesPrimary | StyleFlags_GridSnappingOnRelease;

		Nodes::PushStyleVar(StyleVar_PinLineThickness, 2.5f);
		Nodes::PushStyleVar(StyleVar_NodeCornerRounding, 1.f);
		Nodes::PushStyleVar(StyleVar_PinQuadSideLength, 10.f);
		Nodes::PushStyleVar(StyleVar_NodePadding, v2(10.f, 2.f));

		Nodes::PushStyleVar(StyleVar_NodeBorderThickness, 0.f);

		// Style::PushStyleCompact();
	}

	void PopNodeStyle()
	{
		// Style::PopStyleCompact();
		Nodes::PopStyleVar(4);
	}

	void PushInnerNodeStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const float padding =
		    (settings.GetSpaceHeight(1) - GImGui->FontSize) * 0.5f - settings.verticalPadding;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {style.FramePadding.x, padding});
		ImGui::PushStyleVar(
		    ImGuiStyleVar_ItemSpacing, {style.ItemSpacing.x, settings.verticalPadding});
	}

	void PopInnerNodeStyle()
	{
		ImGui::PopStyleVar(2);
	}

	void DrawContextMenu(AST::Tree& ast)
	{
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
	}

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

		bool wantsToOpenContextMenu = false;
		UI::Begin(graphId.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
		{
			PushNodeStyle();

			Nodes::BeginNodeEditor();

			if (!ImGui::IsAnyItemHovered() && Nodes::IsEditorHovered()
			    && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				wantsToOpenContextMenu = true;
			}


			auto functions = ast.MakeView<CFunctionDecl>();
			for (AST::Id child : *children)
			{
				if (functions.Has(child))
				{
					DrawFunctionNodes(ast, child);
				}
			}

			Nodes::MiniMap(0.2f, MiniMapLocation_TopRight);
			Nodes::EndNodeEditor();
			PopNodeStyle();

			if (wantsToOpenContextMenu)
			{
				ImGui::OpenPopup("GraphContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast);
			UI::End();
		}
	}

	void DrawFunctionEntry(AST::Tree& ast, AST::Id functionId)
	{
		auto* nodes = Nodes::GetCurrentContext();

		Name name;
		if (auto* identifier = ast.TryGet<CIdentifier>(functionId))
		{
			name = identifier->name;
		}

		auto& transform = ast.GetOrAdd<CGraphTransform>(functionId);
		if (UI::IsWindowAppearing()
		    && !(nodes->LeftMouseDragging && Nodes::IsNodeSelected(i32(functionId))))
		{
			SetNodePosition(functionId, transform.position);
		}

		Nodes::PushColorStyle(ColorVar_TitleBar, IM_COL32(191, 56, 11, 255));
		Nodes::BeginNode(i32(functionId));

		Nodes::BeginNodeTitleBar();
		UI::Text(name.ToString().c_str());
		Nodes::EndNodeTitleBar();

		UI::BeginGroup();    // Outputs
		{
			Nodes::BeginOutputAttribute(i32(functionId), PinShape_QuadFilled);
			Nodes::EndOutputAttribute();
		}
		UI::EndGroup();

		Nodes::EndNode();
		Nodes::PopColorStyle();

		if (nodes->LeftMouseDragging || nodes->LeftMouseReleased)
		{
			transform.position = GetNodePosition(functionId);
			Types::Changed(AST::GetLinkedParent(ast, functionId), "Moved nodes");
		}
	}

	void DrawFunctionNodes(AST::Tree& ast, AST::Id functionId)
	{
		DrawFunctionEntry(ast, functionId);
	}

	void DrawCallNode(AST::Id id, StringView name)
	{
		static constexpr Color headerColor = Rift::Style::GetNeutralColor(2);
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Nodes::PushColorStyle(ColorVar_NodeBackground, bodyColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundHovered, bodyColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundSelected, bodyColor.ToPackedABGR());


		Nodes::BeginNode(i32(id));
		{
			ImGui::BeginGroup();
			{
				static constexpr Color color = executionColor;
				Nodes::PushColorStyle(ColorVar_Pin, color.ToPackedABGR());
				Nodes::PushColorStyle(ColorVar_PinHovered, GetHovered(color).ToPackedABGR());

				Nodes::BeginInputAttribute(i32(id), PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndInputAttribute();
				UI::SameLine();
				UI::Text(name.data());
				UI::SameLine();
				Nodes::BeginOutputAttribute(i32(id) + 1, PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutputAttribute();

				Nodes::PopColorStyle();
				Nodes::PopColorStyle();
			}
			ImGui::EndGroup();

			ImRect headerRect{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
			NodeData& node = Nodes::EditorContextGet().Nodes.Pool[GNodes->CurrentNodeIdx];
			headerRect.Expand(node.LayoutStyle.Padding);
			ImGui::ItemAdd(headerRect, ImGui::GetID("title_bar"));
		}
		Nodes::EndNode();

		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
	}

	void DrawBoolLiteralNode(AST::Id id, bool& value)
	{
		static constexpr Color color = GetTypeColor<bool>();
		static const Color darkColor = GetHovered(color);

		Nodes::PushColorStyle(ColorVar_NodeBackground, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundHovered, darkColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundSelected, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeOutline, selectedColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_Pin, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_PinHovered, darkColor.ToPackedABGR());

		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginOutputAttribute(i32(id), PinShape_CircleFilled);
			PushInnerNodeStyle();
			UI::Checkbox("##value", &value);
			PopInnerNodeStyle();
			Nodes::EndOutputAttribute();

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::EditorContextGet()
				    .Nodes.Pool[context->CurrentNodeIdx]
				    .LayoutStyle.BorderThickness = 2.f;
			}
		}
		Nodes::EndNode();

		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
	}

	void DrawStringLiteralNode(AST::Id id, String& value)
	{
		static constexpr Color color = GetTypeColor<String>();
		static const Color darkColor = GetHovered(color);

		Nodes::PushColorStyle(ColorVar_NodeBackground, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundHovered, darkColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeBackgroundSelected, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_NodeOutline, selectedColor.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_Pin, color.ToPackedABGR());
		Nodes::PushColorStyle(ColorVar_PinHovered, darkColor.ToPackedABGR());

		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginOutputAttribute(i32(id), PinShape_CircleFilled);
			PushInnerNodeStyle();

			ImGuiStyle& style     = ImGui::GetStyle();
			const ImVec2 textSize = ImGui::CalcTextSize(value.data(), value.data() + value.size());
			const v2 minSize{settings.GetGridSize() * 4.f, settings.GetGridSize()};
			const v2 size{Math::Max(minSize.x, textSize.x), Math::Max(minSize.y, textSize.y)};
			UI::InputTextMultiline("##value", value, v2(size - settings.GetContentPadding()));
			PopInnerNodeStyle();
			Nodes::EndOutputAttribute();
		}

		const auto* context = Nodes::GetCurrentContext();
		if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
		{
			Nodes::EditorContextGet()
			    .Nodes.Pool[context->CurrentNodeIdx]
			    .LayoutStyle.BorderThickness = 2.f;
		}
		Nodes::EndNode();

		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
		Nodes::PopColorStyle();
	}

	void SetNodePosition(AST::Id id, v2 position)
	{
		position *= settings.GetGridSize();
		Nodes::SetNodeGridSpacePos(i32(id), position);
	}

	v2 GetNodePosition(AST::Id id)
	{
		const ImVec2 pos = Nodes::GetNodeGridSpacePos(i32(id));
		return {pos.x * settings.GetInvGridSize(), pos.y * settings.GetInvGridSize()};
	}
}    // namespace Rift::Graph
