// Copyright 2015-2021 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/GraphColors.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CBoolLiteral.h>
#include <AST/Components/CCallExpr.h>
#include <AST/Components/CCompoundStmt.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CStringLiteral.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Linkage.h>
#include <AST/Statics/STypeList.h>
#include <UI/Nodes.h>
#include <UI/NodesInternal.h>
#include <UI/NodesMiniMap.h>
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

	void DrawContextMenu(AST::Tree& ast, AST::Id typeId)
	{
		if (ImGui::BeginPopup("GraphContextMenu"))
		{
			static ImGuiTextFilter filter;
			if (UI::IsWindowAppearing())
			{
				UI::SetKeyboardFocusHere();
			}
			filter.Draw("##Filter");
			const v2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();
			const v2 gridPos  = Nodes::ScreenToGridPosition(clickPos);

			if (filter.IsActive() || ImGui::TreeNode("Constructors"))
			{
				String makeStr{};
				auto& typeList   = ast.GetStatic<STypeList>();
				auto identifiers = ast.Query<CIdentifier>();
				for (const auto& type : typeList.types)
				{
					if (auto* iden = identifiers.TryGet<CIdentifier>(type.second))
					{
						makeStr.clear();
						Strings::FormatTo(makeStr, "Make {}", iden->name);
						if (filter.PassFilter(makeStr.c_str(), makeStr.c_str() + makeStr.size()))
						{
							if (ImGui::MenuItem(makeStr.c_str()))
							{
								AST::Id newId = Types::CreateLiteral(ast, type.second, typeId);
								if (newId != AST::NoId)
								{
									ast.Add<CGraphTransform>(newId, gridPos);
								}
							}
						}
					}
				}

				if (!filter.IsActive())
				{
					ImGui::TreePop();
				}
			}

			if (filter.IsActive() || ImGui::TreeNode("Functions"))
			{
				auto functions   = ast.Query<CFunctionDecl>();
				auto identifiers = ast.Query<CIdentifier>();
				for (AST::Id functionId : functions)
				{
					if (auto* iden = identifiers.TryGet<CIdentifier>(functionId))
					{
						const String& name = iden->name.ToString();
						if (filter.PassFilter(name.c_str(), name.c_str() + name.size()))
						{
							if (ImGui::MenuItem(name.c_str()))
							{
								AST::Id newId = Types::CreateCall(ast, functionId, typeId);
								if (newId != AST::NoId)
								{
									ast.Add<CGraphTransform>(newId, gridPos);
								}
							}
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

	void DrawFunctionDecls(AST::Tree& ast, TArray<AST::Id>& functionDecls)
	{
		auto functions = ast.Query<CFunctionDecl>();
		for (AST::Id functionId : functionDecls)
		{
			DrawFunctionDecl(ast, functionId);
		}
	}

	void DrawCalls(AST::Tree& ast, TArray<AST::Id>& children)
	{
		auto calls         = ast.Query<CCallExpr>();
		auto functionDecls = ast.Query<CFunctionDecl, CIdentifier>();
		for (AST::Id child : children)
		{
			auto* call = calls.TryGet<CCallExpr>(child);
			if (call)
			{
				StringView title = "Invalid";
				if (ast.IsValid(call->functionId))
				{
					if (auto* iden = functionDecls.TryGet<CIdentifier>(call->functionId))
					{
						title = iden->name.ToString().c_str();
					}
				}
				DrawCallNode(child, title);
			}
		}
	}

	void DrawLiterals(AST::Tree& ast, TArray<AST::Id>& children)
	{
		auto boolLiterals = ast.Query<CBoolLiteral>();
		for (AST::Id child : children)
		{
			if (auto* literal = boolLiterals.TryGet<CBoolLiteral>(child))
			{
				DrawBoolLiteralNode(child, literal->value);
			}
		}

		auto stringLiterals = ast.Query<CStringLiteral>();
		for (AST::Id child : children)
		{
			if (auto* literal = stringLiterals.TryGet<CStringLiteral>(child))
			{
				DrawStringLiteralNode(child, literal->value);
			}
		}
	}

	void DrawStatementLinks(AST::Tree& ast, TArray<AST::Id>& functionDecls)
	{
		auto statements = ast.Query<CStatement>();
		auto compounds  = ast.Query<CCompoundStmt>();
		for (AST::Id functionId : functionDecls)
		{
			TArray<AST::Id>* functionChildren = AST::GetLinked(ast, functionId);
			if (!functionChildren || functionChildren->IsEmpty())
			{
				continue;
			}

			AST::Id compoundId = functionChildren->Last();
			if (compounds.Has(compoundId))
			{
				TArray<AST::Id>* statementChildren = AST::GetLinked(ast, compoundId);
				if (!statementChildren || statementChildren->IsEmpty())
				{
					continue;
				}

				AST::Id outputId = statementChildren->First();

				// Connect function to first statement
				Nodes::Link(i32(functionId), i32(functionId), i32(outputId));

				for (i32 i = 1; i < statementChildren->Size(); ++i)
				{
					AST::Id inputId = (*statementChildren)[i];

					// Note: The id of the link is the destination pin
					Nodes::Link(i32(outputId), i32(outputId), i32(inputId));
					outputId = inputId;
				}
			}
		}
	}

	void DrawTypeGraph(AST::Tree& ast, AST::Id typeId, CTypeEditor& typeEditor)
	{
		TArray<AST::Id>* children = AST::GetLinked(ast, typeId);
		if (!children)
		{
			return;
		}

		typeEditor.layout.BindNextWindowToNode(CTypeEditor::centralNode);
		static String graphId;
		graphId.clear();
		Strings::FormatTo(graphId, "Graph##{}", typeId);

		bool wantsToOpenContextMenu = false;
		UI::Begin(graphId.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
		{
			Nodes::SetEditorContext(&typeEditor.nodesEditor);
			Nodes::BeginNodeEditor();
			PushNodeStyle();

			if (!ImGui::IsAnyItemHovered() && Nodes::IsEditorHovered()
			    && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				wantsToOpenContextMenu = true;
			}

			TArray<AST::Id> functions;
			auto functionsQuery = ast.Query<CFunctionDecl>();
			for (AST::Id childId : *children)
			{
				if (functionsQuery.Has(childId))
				{
					functions.Add(childId);
				}
			}

			DrawFunctionDecls(ast, functions);
			DrawCalls(ast, *children);
			DrawLiterals(ast, *children);

			DrawStatementLinks(ast, functions);

			Nodes::DrawMiniMap(0.2f, Nodes::MiniMapCorner::TopRight);
			PopNodeStyle();
			Nodes::EndNodeEditor();

			Nodes::Id startPin, endPin;
			if (Nodes::IsLinkCreated(startPin, endPin))
			{
				Log::Info("New link!");
				AST::Link(ast, AST::Id(startPin), AST::Id(endPin));
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				// Link to type, meaning disconnected
				AST::Link(ast, typeId, AST::Id(linkId));
			}

			if (wantsToOpenContextMenu)
			{
				ImGui::OpenPopup("GraphContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast, typeId);
			UI::End();
		}
	}

	void DrawFunctionDecl(AST::Tree& ast, AST::Id functionId)
	{
		auto* nodes = Nodes::GetCurrentContext();

		Name name;
		if (auto* identifier = ast.TryGet<CIdentifier>(functionId))
		{
			name = identifier->name;
		}

		auto& transform = ast.GetOrAdd<CGraphTransform>(functionId);
		if (UI::IsWindowAppearing()
		    && !(nodes->leftMouseDragging && Nodes::IsNodeSelected(i32(functionId))))
		{
			SetNodePosition(functionId, transform.position);
		}

		static constexpr Color headerColor = functionColor;
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Nodes::PushStyleColor(ColorVar_NodeBackground, bodyColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundHovered, bodyColor);
		Nodes::PushStyleColor(ColorVar_NodeBackgroundSelected, bodyColor);
		Nodes::PushStyleColor(ColorVar_TitleBar, headerColor);
		Nodes::PushStyleColor(ColorVar_TitleBarHovered, GetHovered(headerColor));
		Nodes::PushStyleColor(ColorVar_TitleBarSelected, headerColor);

		// Nodes::PushStyleColor(ColorVar_TitleBar, Color::FromRGB(191, 56, 11));

		Nodes::BeginNode(i32(functionId));
		{
			Nodes::BeginNodeTitleBar();
			{
				UI::Text(name.ToString().c_str());
				UI::SameLine();

				static constexpr Color color = executionColor;
				Nodes::PushStyleColor(ColorVar_Pin, color);
				Nodes::PushStyleColor(ColorVar_PinHovered, GetHovered(color));
				Nodes::BeginOutput(i32(functionId), PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutput();
				Nodes::PopStyleColor(2);
			}
			Nodes::EndNodeTitleBar();
		}
		Nodes::EndNode();
		Nodes::PopStyleColor(6);

		if (nodes->leftMouseDragging || nodes->leftMouseReleased)
		{
			transform.position = GetNodePosition(functionId);
			Types::Changed(AST::GetLinkedParent(ast, functionId), "Moved nodes");
		}
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

				Nodes::BeginInput(i32(id), PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndInput();
				UI::SameLine();
				UI::Text(name.data());
				UI::SameLine();
				Nodes::BeginOutput(i32(id), PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutput();

				Nodes::PopStyleColor(2);
			}
			Nodes::EndNodeTitleBar();

			static constexpr Color pinColor = GetTypeColor<float>();
			Nodes::PushStyleColor(ColorVar_Pin, pinColor);
			Nodes::PushStyleColor(ColorVar_PinHovered, GetHovered(pinColor));
			Nodes::BeginInput(i32(id) + 2, PinShape_CircleFilled);
			UI::Text("amount");
			Nodes::EndInput();
			Nodes::PopStyleColor(2);

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::GetEditorContext()
				    .nodes.Pool[context->CurrentNodeIdx]
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
			Nodes::BeginOutput(i32(id), PinShape_CircleFilled);
			PushInnerNodeStyle();
			UI::Checkbox("##value", &value);
			PopInnerNodeStyle();
			Nodes::EndOutput();

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::GetEditorContext()
				    .nodes.Pool[context->CurrentNodeIdx]
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
			Nodes::BeginOutput(i32(id), PinShape_CircleFilled);
			PushInnerNodeStyle();
			ImGuiStyle& style     = ImGui::GetStyle();
			const ImVec2 textSize = ImGui::CalcTextSize(value.data(), value.data() + value.size());
			const v2 minSize{settings.GetGridSize() * 4.f, settings.GetGridSize()};
			const v2 size{Math::Max(minSize.x, textSize.x), Math::Max(minSize.y, textSize.y)};
			UI::InputTextMultiline("##value", value, v2(size - settings.GetContentPadding()));
			PopInnerNodeStyle();
			Nodes::EndOutput();

			const auto* context = Nodes::GetCurrentContext();
			if (Nodes::IsNodeSelected(context->CurrentNodeIdx))
			{
				Nodes::GetEditorContext()
				    .nodes.Pool[context->CurrentNodeIdx]
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
