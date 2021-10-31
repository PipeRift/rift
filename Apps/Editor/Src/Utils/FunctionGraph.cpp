// Copyright 2015-2020 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
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
		gridSize                      = size;
		invGridSize                   = 1.f / size;
		Nodes::GetStyle().GridSpacing = size;
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
			const ImVec2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

			if (filter.IsActive() || ImGui::TreeNode("Constructors"))
			{
				String makeStr{};
				auto& typeList   = ast.GetUnique<CTypeListUnique>();
				auto identifiers = ast.MakeView<CIdentifier>();
				for (const auto& type : typeList.types)
				{
					if (auto* iden = identifiers.TryGet<CIdentifier>(type.second))
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

		Nodes::PushStyleColor(ColorVar_TitleBar, Color::FromRGB(191, 56, 11));
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
		Nodes::PopStyleColor();

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
		static constexpr Color headerColor = callColor;
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Nodes::PushStyleColor(ColorVar_NodeBackground, bodyColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundHovered, bodyColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundSelected, bodyColor);
		Nodes::PushStyleColor(ColorVar_TitleBar, headerColor);
		Nodes::PushStyleColor(ColorVar_TitleBarHovered, GetHovered(headerColor));
		Nodes::PushStyleColor(ColorVar_TitleBarSelected, headerColor);
		Nodes::PushStyleColor(ColorVar_NodeOutline, selectedColor);


		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginNodeTitleBar();
			{
				static constexpr Color color = executionColor;
				Nodes::PushStyleColor(ColorVar_Pin, color);
				Nodes::PushStyleColor(ColorVar_PinHovered, GetHovered(color));

				Nodes::BeginInputAttribute(i32(id), PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndInputAttribute();
				UI::SameLine();
				UI::Text(name.data());
				UI::SameLine();
				Nodes::BeginOutputAttribute(i32(id) + 1, PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutputAttribute();

				Nodes::PopStyleColor();
				Nodes::PopStyleColor();
			}
			Nodes::EndNodeTitleBar();


			static constexpr Color pinColor = GetTypeColor<float>();
			Nodes::PushStyleColor(ColorVar_Pin, pinColor);
			Nodes::PushStyleColor(ColorVar_PinHovered, GetHovered(pinColor));
			Nodes::BeginInputAttribute(i32(id) + 2, PinShape_CircleFilled);
			UI::Text("amount");
			Nodes::EndInputAttribute();
			Nodes::PopStyleColor(2);

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::EditorContextGet()
				    .Nodes.Pool[context->CurrentNodeIdx]
				    .LayoutStyle.BorderThickness = 2.f;
			}
		}
		Nodes::EndNode();

		Nodes::PopStyleColor(7);
	}

	void DrawBoolLiteralNode(AST::Id id, bool& value)
	{
		static constexpr Color color = GetTypeColor<bool>();
		static const Color darkColor = GetHovered(color);

		Nodes::PushStyleColor(ColorVar_NodeBackground, color);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundHovered, darkColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundSelected, color);
		Nodes::PushStyleColor(ColorVar_NodeOutline, selectedColor);
		Nodes::PushStyleColor(ColorVar_Pin, color);
		Nodes::PushStyleColor(ColorVar_PinHovered, darkColor);

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

		Nodes::PopStyleColor(6);
	}

	void DrawStringLiteralNode(AST::Id id, String& value)
	{
		static constexpr Color color = GetTypeColor<String>();
		static const Color darkColor = GetHovered(color);

		Nodes::PushStyleColor(ColorVar_NodeBackground, color);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundHovered, darkColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundSelected, color);
		Nodes::PushStyleColor(ColorVar_NodeOutline, selectedColor);
		Nodes::PushStyleColor(ColorVar_Pin, color);
		Nodes::PushStyleColor(ColorVar_PinHovered, darkColor);

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

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::EditorContextGet()
				    .Nodes.Pool[context->CurrentNodeIdx]
				    .LayoutStyle.BorderThickness = 2.f;
			}
		}
		Nodes::EndNode();

		Nodes::PopStyleColor(6);
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
