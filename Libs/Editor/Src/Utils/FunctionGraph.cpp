// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "AST/Components/CParameterDecl.h"
#include "AST/Utils/ExpressionGraph.h"
#include "AST/Utils/StatementGraph.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/EditorStyle.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CExprCall.h>
#include <AST/Components/CExpressionInput.h>
#include <AST/Components/CExpressionOutputs.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CLiteralBool.h>
#include <AST/Components/CLiteralString.h>
#include <AST/Components/CParameterDecl.h>
#include <AST/Components/CStatementInput.h>
#include <AST/Components/CStatementOutputs.h>
#include <AST/Components/CVariableDecl.h>
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

	void BeginNode(AST::Id id)
	{
		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);
		Nodes::BeginNode(i32(id));
	}

	void EndNode()
	{
		// Selection outline
		const auto* context = Nodes::GetCurrentContext();
		if (Nodes::IsNodeSelectedByIdx(context->CurrentNodeIdx))
		{
			Nodes::GetEditorContext()
			    .nodes.Pool[context->CurrentNodeIdx]
			    .LayoutStyle.BorderThickness = 2.f;
		}

		Nodes::EndNode();
		Nodes::PopStyleColor(1);
	}


	namespace Literals
	{
		void DrawBoolNode(AST::Id id, bool& value)
		{
			static constexpr Color color = Style::GetTypeColor<bool>();

			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(id);
			{
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				PushInnerNodeStyle();
				UI::Checkbox("##value", &value);
				PopInnerNodeStyle();
				Nodes::EndOutput();
			}
			EndNode();
			Nodes::PopStyleColor(2);
			Style::PopNodeBackgroundColor();
		}

		void DrawStringNode(AST::Id id, String& value)
		{
			static constexpr Color color = Style::GetTypeColor<String>();

			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(id);
			{
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				PushInnerNodeStyle();
				ImGuiStyle& style = ImGui::GetStyle();
				const ImVec2 textSize =
				    ImGui::CalcTextSize(value.data(), value.data() + value.size());
				const v2 minSize{settings.GetGridSize() * 4.f, settings.GetGridSize()};
				const v2 size{Math::Max(minSize.x, textSize.x), Math::Max(minSize.y, textSize.y)};
				UI::InputTextMultiline("##value", value, v2(size - settings.GetContentPadding()));
				PopInnerNodeStyle();
				Nodes::EndOutput();
			}
			EndNode();

			Nodes::PopStyleColor(2);
			Style::PopNodeBackgroundColor();
		}
	}    // namespace Literals


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

		Style::PushNodeBackgroundColor(bodyColor);
		Style::PushNodeTitleColor(headerColor);
		BeginNode(functionId);
		{
			Nodes::BeginNodeTitleBar();
			{
				UI::Text(name.ToString());
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
						UI::Text(name);

						Nodes::EndOutput();
						Nodes::PopStyleColor(2);
					}
				}
			}
		}
		EndNode();
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();

		if (nodes->leftMouseDragging || nodes->leftMouseReleased)
		{
			transform.position = GetNodePosition(functionId);
			Types::Changed(AST::Hierarchy::GetParent(ast, functionId), "Moved nodes");
		}
	}

	void DrawCallNode(AST::Tree& ast, AST::Id id, StringView name, StringView ownerName)
	{
		static constexpr Color headerColor = Style::callColor;
		static constexpr Color bodyColor{Rift::Style::GetNeutralColor(0)};

		Style::PushNodeBackgroundColor(bodyColor);
		Style::PushNodeTitleColor(headerColor);
		BeginNode(id);
		{
			Nodes::BeginNodeTitleBar();
			{
				static constexpr Color color = Style::executionColor;
				Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
				Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

				Nodes::BeginInput(i32(id), Nodes::PinShape_QuadFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::SameLine();

				UI::BeginGroup();
				UI::TextUnformatted(name.data());
				if (!ownerName.empty())
				{
					// Rift::Style::PushTextColor(Rift::Style::whiteTextColor.Shade(0.3f));
					// UI::Text(ownerName.data());
					// Rift::Style::PopTextColor();
				}
				UI::EndGroup();

				UI::SameLine();
				Nodes::BeginOutput(i32(id), Nodes::PinShape_QuadFilled);
				UI::TextUnformatted("");
				Nodes::EndOutput();

				Nodes::PopStyleColor(2);
			}
			Nodes::EndNodeTitleBar();

			if (const TArray<AST::Id>* children = AST::Hierarchy::GetChildren(ast, id))
			{
				auto callArgsView = ast.Filter<CIdentifier, CExpressionInput, CExpressionOutputs>();

				UI::BeginGroup();    // Inputs
				for (AST::Id inputId : *children)
				{
					if (callArgsView.Has<CExpressionInput>(inputId)
					    && callArgsView.Has<CIdentifier>(inputId))
					{
						auto& name           = callArgsView.Get<CIdentifier>(inputId);
						const Color pinColor = Style::GetTypeColor<float>();
						Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
						Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));
						Nodes::BeginInput(i32(inputId), Nodes::PinShape_CircleFilled);
						UI::Text(name.name.ToString());
						Nodes::EndInput();
						Nodes::PopStyleColor(2);
					}
				}
				UI::EndGroup();
				UI::SameLine();
				UI::BeginGroup();    // Outputs
				for (AST::Id outputId : *children)
				{
					if (callArgsView.Has<CExpressionOutputs>(outputId)
					    && callArgsView.Has<CIdentifier>(outputId))
					{
						auto& name           = callArgsView.Get<CIdentifier>(outputId);
						const Color pinColor = Style::GetTypeColor<float>();
						Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
						Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));
						Nodes::BeginOutput(i32(outputId), Nodes::PinShape_CircleFilled);
						UI::Text(name.name.ToString());
						Nodes::EndOutput();
						Nodes::PopStyleColor(2);
					}
				}
				UI::EndGroup();
			}
		}
		EndNode();
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();
	}


	void Init()
	{
		Nodes::CreateContext();
		settings.SetGridSize(settings.GetGridSize());
	}

	void Shutdown()
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

			if (filter.IsActive() || ImGui::TreeNode("Variables"))
			{
				auto variables   = ast.Filter<CVariableDecl>();
				auto identifiers = ast.Filter<CIdentifier>();
				for (AST::Id variableId : variables)
				{
					if (auto* iden = identifiers.TryGet<CIdentifier>(variableId))
					{
						const String& name = iden->name.ToString();
						if (filter.PassFilter(name.c_str(), name.c_str() + name.size()))
						{
							if (ImGui::MenuItem(name.c_str()))
							{
								AST::Id newId = AST::Functions::AddDeclarationReference(
								    {ast, typeId}, variableId);
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
		auto calls         = ast.Filter<CExprCall>();
		auto functionDecls = ast.Filter<CFunctionDecl, CIdentifier>();

		for (AST::Id child : children)
		{
			auto* call = calls.TryGet<CExprCall>(child);
			if (call)
			{
				StringView functionName = call->functionName.ToString().c_str();
				StringView ownerName;
				if (ast.Get<CType>(typeId).name != call->ownerName)    // If not the same type
				{
					ownerName = call->ownerName.ToString().c_str();
				}
				DrawCallNode(ast, child, functionName, ownerName);
			}
		}
	}

	void DrawLiterals(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		auto boolLiterals = ast.Filter<CLiteralBool>();
		for (AST::Id child : children)
		{
			if (auto* literal = boolLiterals.TryGet<CLiteralBool>(child))
			{
				Literals::DrawBoolNode(child, literal->value);
			}
		}

		auto stringLiterals = ast.Filter<CLiteralString>();
		for (AST::Id child : children)
		{
			if (auto* literal = stringLiterals.TryGet<CLiteralString>(child))
			{
				Literals::DrawStringNode(child, literal->value);
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
					const AST::Id outputPinId = childOutputs->linkPins[i];
					const AST::Id inputNodeId = childOutputs->linkInputNodes[i];
					if (!IsNone(outputPinId) && !IsNone(inputNodeId))
					{
						// NOTE: Input pin ids equal input node ids
						// TODO: Execution pin ids atm are the same as the node id. Implement proper
						// output pin support
						Nodes::Link(i32(inputNodeId), i32(outputPinId), i32(inputNodeId));
					}
				}
			}
		}
		Nodes::PopStyleColor(3);
		Nodes::PopStyleVar();
	}

	void DrawExpressionLinks(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 1.5f);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		auto inputsFilter = ast.Filter<CExpressionInput>();
		TArray<AST::Id> exprInputs;
		AST::Hierarchy::GetChildren(ast, children, exprInputs);
		inputsFilter.FilterIds(exprInputs);

		for (AST::Id inputPinId : exprInputs)
		{
			const auto& input         = inputsFilter.Get<CExpressionInput>(inputPinId);
			const AST::Id outputPinId = input.linkOutputPin;

			if (!IsNone(outputPinId))
			{
				// #TODO: Implement link types
				const Color color = Style::GetTypeColor<float>();
				Nodes::PushStyleColor(Nodes::ColorVar_Link, color);
				Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(color));

				Nodes::Link(i32(inputPinId), i32(outputPinId), i32(inputPinId));

				Nodes::PopStyleColor(2);
			}
		}
		Nodes::PopStyleColor();
		Nodes::PopStyleVar();
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
				DrawExpressionLinks(ast, *children);
			}

			Nodes::DrawMiniMap(0.2f, Nodes::MiniMapCorner::TopRight);
			PopNodeStyle();
			Nodes::EndNodeEditor();

			Nodes::Id outputPin;
			Nodes::Id inputPin;
			if (Nodes::IsLinkCreated(outputPin, inputPin))
			{
				AST::StatementGraph::TryConnect(
				    ast, AST::Id(outputPin), AST::Id(outputPin), AST::Id(inputPin));
				AST::ExpressionGraph::TryConnect(ast, AST::Id(outputPin), AST::Id(inputPin));
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				// linkId is always the outputId
				AST::StatementGraph::Disconnect(ast, AST::Id(linkId));
				AST::ExpressionGraph::Disconnect(ast, AST::Id(linkId));
			}

			if (wantsToOpenContextMenu)
			{
				ImGui::OpenPopup("GraphContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast, typeId);
			UI::End();
		}
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
