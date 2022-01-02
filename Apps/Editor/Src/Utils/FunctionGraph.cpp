// Copyright 2015-2021 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "AST/Components/CParameterDecl.h"
#include "AST/Utils/StatementGraph.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "Utils/EditorColors.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CBoolLiteral.h>
#include <AST/Components/CCallExpr.h>
#include <AST/Components/CExpressionOutputs.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParameterDecl.h>
#include <AST/Components/CStatementInput.h>
#include <AST/Components/CStatementOutputs.h>
#include <AST/Components/CStringLiteral.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/FunctionUtils.h>
#include <AST/Utils/Hierarchy.h>
#include <UI/Nodes.h>
#include <UI/NodesInternal.h>
#include <UI/NodesMiniMap.h>
#include <UI/Style.h>


namespace Rift::Graph
{
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
		Nodes::GetStyle().Flags |= Nodes::StyleFlags_GridLines | Nodes::StyleFlags_GridLinesPrimary
		                         | Nodes::StyleFlags_GridSnappingOnRelease;

		Nodes::PushStyleVar(Nodes::StyleVar_PinLineThickness, 2.5f);
		Nodes::PushStyleVar(Nodes::StyleVar_NodeCornerRounding, 1.f);
		Nodes::PushStyleVar(Nodes::StyleVar_PinQuadSideLength, 10.f);
		Nodes::PushStyleVar(Nodes::StyleVar_NodePadding, v2(10.f, 2.f));

		Nodes::PushStyleVar(Nodes::StyleVar_NodeBorderThickness, 0.f);

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
				auto& typeList = ast.GetStatic<STypes>();
				auto types     = ast.Filter<CType>();
				for (const auto& it : typeList.typesByName)
				{
					if (auto* type = types.TryGet<CType>(it.second))
					{
						makeStr.clear();
						Strings::FormatTo(makeStr, "Make {}", type->name);
						if (filter.PassFilter(makeStr.c_str(), makeStr.c_str() + makeStr.size()))
						{
							if (ImGui::MenuItem(makeStr.c_str()))
							{
								AST::Id newId =
								    AST::Functions::AddLiteral({ast, typeId}, it.second);
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
				auto functions   = ast.Filter<CFunctionDecl>();
				auto identifiers = ast.Filter<CIdentifier>();
				for (AST::Id functionId : functions)
				{
					if (auto* iden = identifiers.TryGet<CIdentifier>(functionId))
					{
						const String& name = iden->name.ToString();
						if (filter.PassFilter(name.c_str(), name.c_str() + name.size()))
						{
							if (ImGui::MenuItem(name.c_str()))
							{
								AST::Id newId = AST::Functions::AddCall({ast, typeId}, functionId);
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

	void DrawFunctionDecls(AST::Tree& ast, const TArray<AST::Id>& functionDecls)
	{
		auto functions = ast.Filter<CFunctionDecl>();
		for (AST::Id functionId : functionDecls)
		{
			DrawFunctionDecl(ast, functionId);
		}
	}

	void DrawCalls(AST::Tree& ast, AST::Id typeId, const TArray<AST::Id>& children)
	{
		auto calls         = ast.Filter<CCallExpr>();
		auto functionDecls = ast.Filter<CFunctionDecl, CIdentifier>();
		for (AST::Id child : children)
		{
			auto* call = calls.TryGet<CCallExpr>(child);
			if (call)
			{
				StringView functionName = call->functionName.ToString().c_str();
				StringView typeName;
				if (ast.Get<CType>(typeId).name != call->typeName)    // If not the same type
				{
					typeName = call->typeName.ToString().c_str();
				}
				DrawCallNode(child, functionName, typeName);
			}
		}
	}

	void DrawLiterals(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		auto boolLiterals = ast.Filter<CBoolLiteral>();
		for (AST::Id child : children)
		{
			if (auto* literal = boolLiterals.TryGet<CBoolLiteral>(child))
			{
				DrawBoolLiteralNode(child, literal->value);
			}
		}

		auto stringLiterals = ast.Filter<CStringLiteral>();
		for (AST::Id child : children)
		{
			if (auto* literal = stringLiterals.TryGet<CStringLiteral>(child))
			{
				DrawStringLiteralNode(child, literal->value);
			}
		}
	}

	void DrawStatementLinks(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 2.f);
		Nodes::PushStyleColor(Nodes::ColorVar_Link, Style::executionColor);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(Style::executionColor));
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		auto stmtOutputs = ast.Filter<CStatementOutputs>();
		for (AST::Id childId : children)
		{
			if (auto* childOutputs = stmtOutputs.TryGet<CStatementOutputs>(childId))
			{
				CheckMsg(childOutputs->linkInputNodes.Size() == childOutputs->linkPins.Size(),
				    "Inputs and pins must match. Graph might be corrupted.");
				for (i32 i = 0; i < childOutputs->linkInputNodes.Size(); ++i)
				{
					const AST::Id outputPinIdx = childOutputs->linkPins[i];
					const AST::Id inputNodeId  = childOutputs->linkInputNodes[i];
					if (!IsNone(outputPinIdx) && !IsNone(inputNodeId))
					{
						// NOTE: Input pin ids equal input node ids
						// TODO: Execution pin ids atm are the same as the node id. Implement proper
						// output pin support
						Nodes::Link(i32(inputNodeId), i32(outputPinIdx), i32(inputNodeId));
					}
				}
			}
		}
		Nodes::PopStyleVar();
		Nodes::PopStyleColor(3);
	}

	void DrawTypeGraph(AST::Tree& ast, AST::Id typeId, CTypeEditor& typeEditor)
	{
		typeEditor.layout.BindNextWindowToNode(CTypeEditor::centralNode, ImGuiCond_Appearing);
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

			if (const TArray<AST::Id>* children = AST::Hierarchy::GetChildren(ast, typeId))
			{
				TArray<AST::Id> functions;
				auto functionsQuery = ast.Filter<CFunctionDecl>();
				for (AST::Id childId : *children)
				{
					if (functionsQuery.Has(childId))
					{
						functions.Add(childId);
					}
				}

				DrawFunctionDecls(ast, functions);
				DrawCalls(ast, typeId, *children);
				DrawLiterals(ast, *children);

				DrawStatementLinks(ast, *children);
			}

			Nodes::DrawMiniMap(0.2f, Nodes::MiniMapCorner::TopRight);
			PopNodeStyle();
			Nodes::EndNodeEditor();

			Nodes::Id outputPin;
			Nodes::Id inputPin;
			if (Nodes::IsLinkCreated(outputPin, inputPin))
			{
				AST::StatementGraph::Connect(
				    ast, AST::Id{outputPin}, AST::Id{outputPin}, AST::Id{inputPin});
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				// linkId is always the outputId
				AST::StatementGraph::Disconnect(ast, AST::Id(linkId));
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
		auto* nodes      = Nodes::GetCurrentContext();
		auto identifiers = ast.Filter<CIdentifier>();

		Name name;
		if (auto* identifier = identifiers.TryGet<CIdentifier>(functionId))
		{
			name = identifier->name;
		}

		auto& transform = ast.GetOrAdd<CGraphTransform>(functionId);
		if (UI::IsWindowAppearing()
		    && !(nodes->leftMouseDragging && Nodes::IsNodeSelected(i32(functionId))))
		{
			SetNodePosition(functionId, transform.position);
		}

		static constexpr Color headerColor = Style::functionColor;
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackground, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundHovered, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundSelected, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBar, headerColor);
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarHovered, Style::Hovered(headerColor));
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarSelected, headerColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);

		// Nodes::PushStyleColor(ColorVar_TitleBar, Color::FromRGB(191, 56, 11));

		Nodes::BeginNode(i32(functionId));
		{
			Nodes::BeginNodeTitleBar();
			{
				UI::Text(name.ToString().c_str());
				UI::SameLine();

				static constexpr Color color = Style::executionColor;
				Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
				Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));
				Nodes::BeginOutput(i32(functionId), Nodes::PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutput();
				Nodes::PopStyleColor(2);
			}
			Nodes::EndNodeTitleBar();


			if (const TArray<AST::Id>* children = AST::Hierarchy::GetChildren(ast, functionId))
			{
				auto inputParameters = ast.Filter<CParameterDecl, CExpressionOutputs>();
				for (AST::Id childId : *children)
				{
					if (inputParameters.Has(childId))
					{
						auto& param = inputParameters.Get<CParameterDecl>(childId);

						const Color pinColor = Style::GetTypeColor(ast, param.typeId);
						Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
						Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));

						Nodes::BeginOutput(i32(childId), Nodes::PinShape_CircleFilled);

						auto* ident = identifiers.TryGet<CIdentifier>(childId);
						String name = ident ? ident->name.ToString() : "";
						UI::Text(name.c_str());

						Nodes::EndOutput();
						Nodes::PopStyleColor(2);
					}
				}
			}

			if (Nodes::IsNodeSelected(i32(functionId)))
			{
				const auto* context = Nodes::GetCurrentContext();
				Nodes::GetEditorContext()
				    .nodes.Pool[context->CurrentNodeIdx]
				    .LayoutStyle.BorderThickness = 2.f;
			}
		}
		Nodes::EndNode();
		Nodes::PopStyleColor(6);

		if (nodes->leftMouseDragging || nodes->leftMouseReleased)
		{
			transform.position = GetNodePosition(functionId);
			Types::Changed(AST::Hierarchy::GetParent(ast, functionId), "Moved nodes");
		}
	}

	void DrawCallNode(AST::Id id, StringView name, StringView typeName)
	{
		static constexpr Color headerColor = Style::callColor;
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackground, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundHovered, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundSelected, bodyColor);
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBar, headerColor);
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarHovered, Style::Hovered(headerColor));
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarSelected, headerColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);

		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginNodeTitleBar();
			{
				static constexpr Color color = Style::executionColor;
				Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
				Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

				Nodes::BeginInput(i32(id), Nodes::PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndInput();
				UI::SameLine();

				UI::BeginGroup();
				UI::Text(name.data());
				if (!typeName.empty())
				{
					// Rift::Style::PushTextColor(Rift::Style::whiteTextColor.Shade(0.3f));
					// UI::Text(typeName.data());
					// Rift::Style::PopTextColor();
				}
				UI::EndGroup();

				UI::SameLine();
				Nodes::BeginOutput(i32(id), Nodes::PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutput();

				Nodes::PopStyleColor(2);
			}
			Nodes::EndNodeTitleBar();

			UI::BeginGroup();    // Inputs
			const Color pinColor = Style::GetTypeColor<float>();
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));
			// Nodes::BeginInput(i32(id) + 2, PinShape_CircleFilled);
			// UI::Text("amount");
			// Nodes::EndInput();
			Nodes::PopStyleColor(2);
			UI::EndGroup();

			if (Nodes::IsNodeSelected(i32(id)))
			{
				const auto* context = Nodes::GetCurrentContext();
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
		static constexpr Color color = Style::GetTypeColor<bool>();
		static const Color darkColor = Style::Hovered(color);

		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackground, color);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundHovered, darkColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundSelected, color);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, darkColor);

		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
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
		static constexpr Color color = Style::GetTypeColor<String>();
		static const Color darkColor = Style::Hovered(color);

		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackground, color);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundHovered, darkColor);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundSelected, color);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, darkColor);

		Nodes::BeginNode(i32(id));
		{
			Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
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
