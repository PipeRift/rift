// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "AST/Components/CParameterDecl.h"
#include "AST/Utils/ExpressionGraph.h"
#include "AST/Utils/StatementGraph.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/EditorStyle.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CBoolLiteral.h>
#include <AST/Components/CCallExpr.h>
#include <AST/Components/CExpressionInput.h>
#include <AST/Components/CExpressionOutputs.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParameterDecl.h>
#include <AST/Components/CReturnExpr.h>
#include <AST/Components/CStatementIf.h>
#include <AST/Components/CStatementInput.h>
#include <AST/Components/CStatementOutputs.h>
#include <AST/Components/CStringLiteral.h>
#include <AST/Components/CVariableDecl.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Filtering.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/FunctionUtils.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TransactionUtils.h>
#include <UI/Style.h>
#include <Utils/Nodes.h>
#include <Utils/NodesInternal.h>
#include <Utils/NodesMiniMap.h>


namespace Rift::Graph
{
	static CGraphTransform* currentNodeTransform = nullptr;

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

	void BeginNode(TAccessRef<TWrite<CGraphTransform>> access, AST::Id id)
	{
		currentNodeTransform = &access.GetOrAdd<CGraphTransform>(id);
		if (UI::IsWindowAppearing()
		    && !(Nodes::GetCurrentContext()->leftMouseDragging && Nodes::IsNodeSelected(id)))
		{
			SetNodePosition(id, currentNodeTransform->position);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, Style::selectedColor);
		Nodes::BeginNode(id);
	}

	void EndNode(const TransactionAccess& access)
	{
		// Selection outline
		const auto* context = Nodes::GetCurrentContext();
		if (Nodes::IsNodeSelected(context->currentNodeId))
		{
			Nodes::GetEditorContext().nodes[context->currentNodeId].LayoutStyle.BorderThickness =
			    2.f;
		}
		Nodes::EndNode();
		Nodes::PopStyleColor(1);

		if (currentNodeTransform && context->leftMouseReleased)
		{
			const AST::Id id = context->currentNodeId;
			v2 newPosition   = GetNodePosition(id);
			if (!newPosition.Equals(currentNodeTransform->position, 0.1f))
			{
				ScopedChange(access, id);
				currentNodeTransform->position = newPosition;
			}
		}
		currentNodeTransform = nullptr;
	}

	void PushExecutionPinStyle()
	{
		static constexpr Color color = Style::executionColor;
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));
	}

	void PopExecutionPinStyle()
	{
		Nodes::PopStyleColor(2);
	}

	void DrawLiteralBool(AST::Tree& ast, AST::Id id, bool& value)
	{
		static constexpr Color color = Style::GetTypeColor<bool>();

		Style::PushNodeBackgroundColor(color);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

		BeginNode(ast, id);
		{
			Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
			PushInnerNodeStyle();
			UI::Checkbox("##value", &value);
			PopInnerNodeStyle();
			Nodes::EndOutput();
		}
		EndNode(ast);
		Nodes::PopStyleColor(2);
		Style::PopNodeBackgroundColor();
	}

	void DrawLiteralString(AST::Tree& ast, AST::Id id, String& value)
	{
		static constexpr Color color = Style::GetTypeColor<String>();

		Style::PushNodeBackgroundColor(color);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

		BeginNode(ast, id);
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
		}
		EndNode(ast);

		Nodes::PopStyleColor(2);
		Style::PopNodeBackgroundColor();
	}


	void DrawFunctionDecl(AST::Tree& ast, AST::Id functionId)
	{
		auto identifiers = ast.Filter<CIdentifier>();

		Name name;
		if (auto* identifier = identifiers.TryGet<CIdentifier>(functionId))
		{
			name = identifier->name;
		}

		Style::PushNodeBackgroundColor(Rift::Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::functionColor);
		BeginNode(ast, functionId);
		{
			Nodes::BeginNodeTitleBar();
			{
				UI::Text(name.ToString());
				UI::SameLine();

				PushExecutionPinStyle();
				Nodes::BeginOutput(i32(functionId), Nodes::PinShape_QuadFilled);
				UI::Text("");
				Nodes::EndOutput();
				PopExecutionPinStyle();
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
		EndNode(ast);
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();
	}

	void DrawReturnNode(
	    TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef>
	        access,
	    AST::Id id)
	{
		Style::PushNodeBackgroundColor(Rift::Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::returnColor);
		BeginNode(access, id);
		{
			Nodes::BeginNodeTitleBar();
			{
				PushExecutionPinStyle();
				Nodes::BeginInput(i32(id), Nodes::PinShape_QuadFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::SameLine();
				PopExecutionPinStyle();

				UI::BeginGroup();
				UI::TextUnformatted("Return");
				UI::EndGroup();
			}
			Nodes::EndNodeTitleBar();
		}
		EndNode(access);
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();
	}

	void DrawCallNode(AST::Tree& ast, AST::Id id, StringView name, StringView ownerName)
	{
		Style::PushNodeBackgroundColor(Rift::Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::callColor);
		BeginNode(ast, id);
		{
			Nodes::BeginNodeTitleBar();
			{
				PushExecutionPinStyle();

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

				PopExecutionPinStyle();
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
		EndNode(ast);
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();
	}

	void DrawIf(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>, CChild,
	                CFileRef, CStatementOutputs, CParent>
	                access,
	    AST::Id id)
	{
		Style::PushNodeBackgroundColor(Rift::Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::flowColor);
		BeginNode(access, id);
		{
			Nodes::BeginNodeTitleBar();
			{
				PushExecutionPinStyle();
				Nodes::BeginInput(i32(id), Nodes::PinShape_QuadFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::SameLine();

				UI::BeginGroup();
				UI::TextUnformatted("if");
				UI::EndGroup();

				UI::SameLine();
				UI::BeginGroup();
				{
					TArray<AST::Id> statementOutputs;
					AST::Hierarchy::GetChildren(access, id, statementOutputs);
					Nodes::BeginOutput(i32(statementOutputs[0]), Nodes::PinShape_QuadFilled);
					UI::TextUnformatted("true");
					Nodes::EndOutput();
					Nodes::BeginOutput(i32(statementOutputs[1]), Nodes::PinShape_QuadFilled);
					UI::TextUnformatted("false");
					Nodes::EndOutput();
				}
				UI::EndGroup();

				PopExecutionPinStyle();
			}
			Nodes::EndNodeTitleBar();
		}
		EndNode(access);
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
	void DrawNodeContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id nodeId)
	{
		if (ast.Has<CFunctionDecl>(nodeId))
		{
			if (UI::MenuItem("Add return node"))
			{
				AST::Functions::AddReturn({ast, typeId});
			}
			UI::Separator();
		}
		if (UI::MenuItem("Delete")) {}
	}

	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId)
	{
		static ImGuiTextFilter filter;
		if (UI::IsWindowAppearing())
		{
			UI::SetKeyboardFocusHere();
		}
		filter.Draw("##Filter");
		const v2 clickPos = UI::GetMousePosOnOpeningCurrentPopup();
		const v2 gridPos  = Nodes::ScreenToGridPosition(clickPos);

		if (filter.IsActive() || UI::TreeNode("Flow"))
		{
			if (filter.PassFilter("Return") && UI::MenuItem("Return"))
			{
				AST::Functions::AddReturn({ast, typeId});
			}
			if (filter.PassFilter("If") && UI::MenuItem("If"))
			{
				AST::Functions::AddIf({ast, typeId});
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Constructors"))
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
						if (UI::MenuItem(makeStr.c_str()))
						{
							AST::Id newId = AST::Functions::AddLiteral({ast, typeId}, it.second);
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
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Variables"))
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
						if (UI::MenuItem(name.c_str()))
						{
							AST::Id newId =
							    AST::Functions::AddDeclarationReference({ast, typeId}, variableId);
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
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Functions"))
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
						if (UI::MenuItem(name.c_str()))
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
	}

	void DrawContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId)
	{
		if (ImGui::BeginPopup("GraphContextMenu"))
		{
			if (IsNone(hoveredNodeId))
			{
				DrawGraphContextMenu(ast, typeId);
			}
			else
			{
				DrawNodeContextMenu(ast, typeId, hoveredNodeId);
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

	void DrawReturns(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>,
	                     CChild, CFileRef, CReturnExpr>
	                     access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : GetIf<CReturnExpr>(access, children))
		{
			DrawReturnNode(access, id);
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
		for (AST::Id id : AST::GetIf<CBoolLiteral>(ast, children))
		{
			DrawLiteralBool(ast, id, ast.Get<CBoolLiteral>(id).value);
		}

		for (AST::Id id : AST::GetIf<CStringLiteral>(ast, children))
		{
			DrawLiteralString(ast, id, ast.Get<CStringLiteral>(id).value);
		}
	}

	void DrawIfs(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>, CChild,
	                 CFileRef, CStatementIf, CStatementOutputs, CParent>
	                 access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : AST::GetIf<CStatementIf>(access, children))
		{
			DrawIf(access, id);
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
						// TODO: Execution pin ids atm are the same as the node id. Implement
						// proper output pin support
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

				// Nodes
				DrawFunctionDecls(ast, functions);
				DrawReturns(ast, *children);
				DrawCalls(ast, typeId, *children);
				DrawLiterals(ast, *children);

				DrawIfs(ast, *children);

				// Links
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
				AST::StatementGraph::TryConnect(ast, AST::Id(outputPin), AST::Id(inputPin));
				AST::ExpressionGraph::TryConnect(ast, AST::Id(outputPin), AST::Id(inputPin));
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				// linkId is always the outputId
				AST::StatementGraph::Disconnect(ast, AST::Id(linkId));
				AST::ExpressionGraph::Disconnect(ast, AST::Id(linkId));
			}

			static AST::Id contextNodeId = AST::NoId;
			if (wantsToOpenContextMenu)
			{
				contextNodeId = Nodes::GetNodeHovered();
				ImGui::OpenPopup("GraphContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast, typeId, contextNodeId);
			UI::End();
		}
	}

	void SetNodePosition(AST::Id id, v2 position)
	{
		position *= settings.GetGridSize();
		Nodes::SetNodeGridSpacePos(id, position);
	}

	v2 GetNodePosition(AST::Id id)
	{
		const v2 pos = Nodes::GetNodeGridSpacePos(id);
		return v2{pos.x * settings.GetInvGridSize(), pos.y * settings.GetInvGridSize()}.Floor();
	}
}    // namespace Rift::Graph
