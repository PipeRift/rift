// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionUtils.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprBinaryOperator.h>
#include <AST/Components/CExprCall.h>
#include <AST/Components/CExprDeclRef.h>
#include <AST/Components/CExprInput.h>
#include <AST/Components/CExprOutputs.h>
#include <AST/Components/CExprReturn.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CExprUnaryOperator.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CLiteralBool.h>
#include <AST/Components/CLiteralFloating.h>
#include <AST/Components/CLiteralIntegral.h>
#include <AST/Components/CLiteralString.h>
#include <AST/Components/CStmtIf.h>
#include <AST/Components/CStmtInput.h>
#include <AST/Components/CStmtOutputs.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Filtering.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/ExpressionGraph.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/StatementGraph.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
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
		auto* context        = Nodes::GetCurrentContext();
		if (UI::IsWindowAppearing()
		    || (!context->leftMouseDragging && !context->leftMouseReleased
		        && !Nodes::IsNodeSelected(id)))
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

	void DrawLiteralIntegral(AST::Tree& ast, AST::Id id, CLiteralIntegral& value)
	{
		const bool isSigned = value.IsSigned();
		const Color color   = isSigned ? Style::GetTypeColor<i32>() : Style::GetTypeColor<u32>();

		Style::PushNodeBackgroundColor(color);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

		BeginNode(ast, id);
		{
			Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
			PushInnerNodeStyle();
			UI::SetNextItemWidth(Math::Max(settings.GetGridSize() * 4.f, 30.f));
			switch (value.type)
			{
				case IntegralType::S8:
					UI::InputScalar("##value", ImGuiDataType_S8, &value.value);
					break;
				case IntegralType::S16:
					UI::InputScalar("##value", ImGuiDataType_S16, &value.value);
					break;
				case IntegralType::S32:
					UI::InputScalar("##value", ImGuiDataType_S32, &value.value);
					break;
				case IntegralType::S64:
					UI::InputScalar("##value", ImGuiDataType_S64, &value.value);
					break;
				case IntegralType::U8:
					UI::InputScalar("##value", ImGuiDataType_U8, &value.value);
					break;
				case IntegralType::U16:
					UI::InputScalar("##value", ImGuiDataType_U16, &value.value);
					break;
				case IntegralType::U32:
					UI::InputScalar("##value", ImGuiDataType_U32, &value.value);
					break;
				case IntegralType::U64:
					UI::InputScalar("##value", ImGuiDataType_U64, &value.value);
					break;
			}
			PopInnerNodeStyle();
			Nodes::EndOutput();
		}
		EndNode(ast);
		Nodes::PopStyleColor(2);
		Style::PopNodeBackgroundColor();
	}

	void DrawLiteralFloating(AST::Tree& ast, AST::Id id, CLiteralFloating& value)
	{
		const bool isDouble = value.type == FloatingType::F64;
		const Color color = isDouble ? Style::GetTypeColor<double>() : Style::GetTypeColor<float>();

		Style::PushNodeBackgroundColor(color);
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

		BeginNode(ast, id);
		{
			Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
			PushInnerNodeStyle();
			UI::SetNextItemWidth(Math::Max(settings.GetGridSize() * 4.f, 30.f));
			if (isDouble)
			{
				UI::InputFloat("##value", reinterpret_cast<float*>(&value.value));
			}
			else
			{
				UI::InputDouble("##value", &value.value);
			}
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
				auto inputParameters = ast.Filter<CExprType, CExprOutputs>();
				for (AST::Id childId : *children)
				{
					if (inputParameters.Has(childId))
					{
						auto& type = inputParameters.Get<CExprType>(childId);

						const Color pinColor = Style::GetTypeColor(ast, type.id);
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
				auto callArgsView = ast.Filter<CIdentifier, CExprInput, CExprOutputs>();

				UI::BeginGroup();    // Inputs
				for (AST::Id inputId : *children)
				{
					if (callArgsView.Has<CExprInput>(inputId)
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
					if (callArgsView.Has<CExprOutputs>(outputId)
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
	void DrawNodeContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> nodeIds)
	{
		Check(!nodeIds.IsEmpty());

		AST::Id firstNodeId = nodeIds[0];

		if (nodeIds.Size() == 1 && ast.Has<CDeclFunction>(firstNodeId))
		{
			if (UI::MenuItem("Add return node"))
			{
				AST::Id newId = AST::Functions::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CGraphTransform>(firstNodeId).position;
					ast.Add<CGraphTransform>(newId, position + v2{10.f, 0.f});

					AST::StatementGraph::TryConnect(ast, firstNodeId, newId);
				}
			}
			UI::Separator();
		}
		if (UI::MenuItem("Delete"))
		{
			AST::Functions::RemoveNodes(ast, nodeIds);
		}
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
		const v2 gridPos  = GetGridPosition(clickPos).Floor();

		if (filter.IsActive() || UI::TreeNode("Flow"))
		{
			if (filter.PassFilter("Return") && UI::MenuItem("Return"))
			{
				AST::Id newId = AST::Functions::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					ast.Add<CGraphTransform>(newId, gridPos);
				}
			}
			if (filter.PassFilter("If") && UI::MenuItem("If"))
			{
				AST::Id newId = AST::Functions::AddIf({ast, typeId});
				if (!IsNone(newId))
				{
					ast.Add<CGraphTransform>(newId, gridPos);
				}
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Operators"))
		{
			static String name;
			// Unary operators
			for (auto type : Refl::GetEnumValues<UnaryOperatorType>())
			{
				StringView shortName = Functions::GetUnaryOperatorName(type);
				StringView longName  = Functions::GetUnaryOperatorLongName(type);
				name.clear();
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (filter.PassFilter(name.data(), name.data() + name.size())
				    && UI::MenuItem(name.data()))
				{
					AST::Id newId = AST::Functions::AddUnaryOperator({ast, typeId}, type);
					if (!IsNone(newId))
					{
						ast.Add<CGraphTransform>(newId, gridPos);
					}
				}
			}
			// Binary operators
			for (auto type : Refl::GetEnumValues<BinaryOperatorType>())
			{
				StringView shortName = Functions::GetBinaryOperatorName(type);
				StringView longName  = Functions::GetBinaryOperatorLongName(type);
				name.clear();
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (filter.PassFilter(name.data(), name.data() + name.size())
				    && UI::MenuItem(name.data()))
				{
					AST::Id newId = AST::Functions::AddBinaryOperator({ast, typeId}, type);
					if (!IsNone(newId))
					{
						ast.Add<CGraphTransform>(newId, gridPos);
					}
				}
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
							if (!IsNone(newId))
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
			auto variables   = ast.Filter<CDeclVariable>();
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
							if (!IsNone(newId))
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
			auto functions   = ast.Filter<CDeclFunction>();
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
							if (!IsNone(newId))
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
			else if (Nodes::IsNodeSelected(hoveredNodeId))
			{
				DrawNodeContextMenu(ast, typeId, Nodes::GetSelectedNodes());
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
		for (AST::Id functionId : functionDecls)
		{
			DrawFunctionDecl(ast, functionId);
		}
	}

	void DrawReturns(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>,
	                     CChild, CFileRef, CExprReturn>
	                     access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : GetIf<CExprReturn>(access, children))
		{
			DrawReturnNode(access, id);
		}
	}

	void DrawCalls(AST::Tree& ast, AST::Id typeId, const TArray<AST::Id>& children)
	{
		auto calls = ast.Filter<CExprCall>();
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
		for (AST::Id id : AST::GetIf<CLiteralBool>(ast, children))
		{
			DrawLiteralBool(ast, id, ast.Get<CLiteralBool>(id).value);
		}

		for (AST::Id id : AST::GetIf<CLiteralIntegral>(ast, children))
		{
			DrawLiteralIntegral(ast, id, ast.Get<CLiteralIntegral>(id));
		}

		for (AST::Id id : AST::GetIf<CLiteralFloating>(ast, children))
		{
			DrawLiteralFloating(ast, id, ast.Get<CLiteralFloating>(id));
		}

		for (AST::Id id : AST::GetIf<CLiteralString>(ast, children))
		{
			DrawLiteralString(ast, id, ast.Get<CLiteralString>(id).value);
		}
	}

	void DrawVariableRefs(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		String name;
		for (AST::Id id : AST::GetIf<CExprDeclRefId>(ast, children))
		{
			AST::Id typeId     = AST::NoId;
			AST::Id variableId = ast.Get<const CExprDeclRefId>(id).declarationId;
			if (ast.IsValid(variableId))
			{
				typeId = ast.Get<const CDeclVariable>(variableId).typeId;
			}

			const Color color = Style::GetTypeColor(ast, typeId);
			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(ast, id);
			{
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				PushInnerNodeStyle();
				StringView name = "Invalid";
				if (const auto* identifier = ast.IsValid(variableId)
				                               ? ast.TryGet<const CIdentifier>(variableId)
				                               : nullptr)
				{
					name = identifier->name.ToString();
				}
				UI::Text(name);
				PopInnerNodeStyle();
				Nodes::EndOutput();
			}
			EndNode(ast);
			Nodes::PopStyleColor(2);
			Style::PopNodeBackgroundColor();
		}
	}

	void DrawIfs(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>, TWrite<CFileDirty>, CChild,
	                 CFileRef, CStmtIf, CStmtOutputs, CParent>
	                 access,
	    const TArray<AST::Id>& children)
	{
		Style::PushNodeBackgroundColor(Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::flowColor);
		TArray<AST::Id> pinIds;
		for (AST::Id id : AST::GetIf<CStmtIf>(access, children))
		{
			BeginNode(access, id);
			{
				pinIds.Empty(false);
				AST::Hierarchy::GetChildren(access, id, pinIds);

				Nodes::BeginNodeTitleBar();
				{
					if (!Ensure(pinIds.Size() >= 3))
					{
						continue;
					}
					UI::BeginGroup();
					{
						PushExecutionPinStyle();
						Nodes::BeginInput(i32(id), Nodes::PinShape_QuadFilled);
						UI::TextUnformatted("");
						Nodes::EndInput();
						PopExecutionPinStyle();

						const Color pinColor = Style::GetTypeColor<bool>();
						Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
						Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));

						Nodes::BeginInput(i32(pinIds[0]), Nodes::PinShape_CircleFilled);
						UI::TextUnformatted("");
						Nodes::EndInput();
						Nodes::PopStyleColor(2);
					}
					UI::EndGroup();
					UI::SameLine();

					UI::BeginGroup();
					UI::TextUnformatted("if");
					UI::EndGroup();

					UI::SameLine();
					UI::BeginGroup();
					{
						PushExecutionPinStyle();
						Nodes::BeginOutput(i32(pinIds[1]), Nodes::PinShape_QuadFilled);
						UI::TextUnformatted("true");
						Nodes::EndOutput();
						Nodes::BeginOutput(i32(pinIds[2]), Nodes::PinShape_QuadFilled);
						UI::TextUnformatted("false");
						Nodes::EndOutput();
						PopExecutionPinStyle();
					}
					UI::EndGroup();
				}
				Nodes::EndNodeTitleBar();
			}
			EndNode(access);
		}
		Style::PopNodeTitleColor();
		Style::PopNodeBackgroundColor();
	}

	void DrawUnaryOperators(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>,
	                            TWrite<CFileDirty>, CChild, CParent, CFileRef, CExprUnaryOperator>
	                            access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : AST::GetIf<CExprUnaryOperator>(access, children))
		{
			static constexpr Color color = Style::GetNeutralColor(0);

			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(access, id);
			{
				Nodes::BeginInput(i32(id), Nodes::PinShape_CircleFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::SameLine();

				const auto& op       = access.Get<const CExprUnaryOperator>(id);
				StringView shortName = Functions::GetUnaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				UI::TextUnformatted("");
				Nodes::EndOutput();
			}
			EndNode(access);

			Nodes::PopStyleColor(2);
			Style::PopNodeBackgroundColor();
		}
	}

	void DrawBinaryOperators(TAccessRef<TWrite<CGraphTransform>, TWrite<CChanged>,
	                             TWrite<CFileDirty>, CChild, CParent, CFileRef, CExprBinaryOperator>
	                             access,
	    const TArray<AST::Id>& children)
	{
		TArray<AST::Id> pinIds;
		for (AST::Id id : AST::GetIf<CExprBinaryOperator>(access, children))
		{
			static constexpr Color color = Style::GetNeutralColor(0);

			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(access, id);
			{
				pinIds.Empty(false);
				AST::Hierarchy::GetChildren(access, id, pinIds);
				if (!Ensure(pinIds.Size() >= 2))
				{
					continue;
				}
				UI::BeginGroup();
				Nodes::BeginInput(i32(pinIds[0]), Nodes::PinShape_CircleFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				Nodes::BeginInput(i32(pinIds[1]), Nodes::PinShape_CircleFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::EndGroup();
				UI::SameLine();

				const auto& op       = access.Get<const CExprBinaryOperator>(id);
				StringView shortName = Functions::GetBinaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				UI::TextUnformatted("");
				Nodes::EndOutput();
			}
			EndNode(access);

			Nodes::PopStyleColor(2);
			Style::PopNodeBackgroundColor();
		}
	}

	void DrawStatementLinks(
	    TAccessRef<CParent, CStmtOutputs>& access, const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 2.f);
		Nodes::PushStyleColor(Nodes::ColorVar_Link, Style::executionColor);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(Style::executionColor));
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		for (AST::Id outputId : GetIf<CStmtOutputs>(access, children))
		{
			if (const auto* outputs = access.TryGet<const CStmtOutputs>(outputId))
			{
				CheckMsg(childOutputs->linkInputNodes.Size() == outputs->linkPins.Size(),
				    "Inputs and pins must match. Graph might be corrupted.");
				for (i32 i = 0; i < outputs->linkInputNodes.Size(); ++i)
				{
					const AST::Id outputPinId = outputs->linkPins[i];
					const AST::Id inputNodeId = outputs->linkInputNodes[i];
					if (access.IsValid(outputPinId) && access.IsValid(inputNodeId))
					{
						// NOTE: Input pin ids equal input node ids
						// TODO: Execution pin ids atm are the same as the node id.
						// Implement proper output pin support
						Nodes::Link(i32(inputNodeId), i32(outputPinId), i32(inputNodeId));
					}
				}
			}
		}
		Nodes::PopStyleColor(3);
		Nodes::PopStyleVar();
	}

	void DrawExpressionLinks(
	    TAccessRef<CParent, CExprInput, CExprType>& access, const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 1.5f);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		TArray<AST::Id> exprInputs;
		exprInputs.Append(children);    // Include nodes as possible inputs
		AST::Hierarchy::GetChildren(access, children, exprInputs);
		AST::RemoveIfNot<CExprInput>(access, exprInputs);
		for (AST::Id inputId : exprInputs)
		{
			const auto& input      = access.Get<const CExprInput>(inputId);
			const AST::Id outputId = input.linkOutputPin;
			if (!access.IsValid(outputId))
			{
				continue;
			}


			AST::Id typeId = AST::NoId;
			if (const auto* type = access.TryGet<const CExprType>(outputId))
			{
				typeId = type->id;
			}
			if (IsNone(typeId))
			{
				if (const auto* type = access.TryGet<const CExprType>(inputId))
				{
					typeId = type->id;
				}
			}

			const Color color = Style::GetTypeColor(access.GetAST(), typeId);
			Nodes::PushStyleColor(Nodes::ColorVar_Link, color);
			Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(color));

			Nodes::Link(i32(inputId), i32(outputId), i32(inputId));

			Nodes::PopStyleColor(2);
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
				auto functionsQuery = ast.Filter<CDeclFunction>();
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
				DrawVariableRefs(ast, *children);

				DrawIfs(ast, *children);
				DrawUnaryOperators(ast, *children);
				DrawBinaryOperators(ast, *children);

				// Links
				DrawStatementLinks(ast, *children);
				DrawExpressionLinks(ast, *children);

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					AST::Functions::RemoveNodes(ast, Nodes::GetSelectedNodes());
				}
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


	v2 GetGridPosition(v2 screenPosition)
	{
		return Nodes::ScreenToGridPosition(screenPosition) * settings.GetInvGridSize();
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
