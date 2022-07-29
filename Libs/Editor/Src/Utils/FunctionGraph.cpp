// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprBinaryOperator.h>
#include <AST/Components/CExprCall.h>
#include <AST/Components/CExprDeclRef.h>
#include <AST/Components/CExprInputs.h>
#include <AST/Components/CExprOutputs.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CExprUnaryOperator.h>
#include <AST/Components/CLiteralBool.h>
#include <AST/Components/CLiteralFloating.h>
#include <AST/Components/CLiteralIntegral.h>
#include <AST/Components/CLiteralString.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/CStmtIf.h>
#include <AST/Components/CStmtInput.h>
#include <AST/Components/CStmtOutputs.h>
#include <AST/Components/CStmtReturn.h>
#include <AST/Components/CType.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <Pipe/ECS/Filtering.h>
#include <UI/Style.h>
#include <Utils/Nodes.h>
#include <Utils/NodesInternal.h>
#include <Utils/NodesMiniMap.h>


namespace rift::Graph
{
	static CNodePosition* currentNodeTransform = nullptr;

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

	v2 Settings::GetGridPosition(v2 screenPosition) const
	{
		return Nodes::ScreenToGridPosition(screenPosition) * GetInvGridSize();
	}

	void DrawInputs(TAccessRef<CInvalid, CExprType, CNamespace> access, const CExprInputs& inputs)
	{
		for (AST::Id pinId : inputs.pinIds)
		{
			if (!access.IsValid(pinId))
			{
				continue;
			}

			AST::Id typeId       = AST::NoId;
			Color pinColor       = Style::GetTypeColor<void>();
			const bool isInvalid = access.Has<CInvalid>(pinId);
			if (isInvalid)
			{
				Style::PushTextColor(Style::invalidColor);
				pinColor = Style::invalidColor;
			}
			else if (auto* type = access.TryGet<const CExprType>(pinId))
			{
				typeId = type->id;
				pinColor =
				    Style::GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), typeId);
			}
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));

			Nodes::BeginInput(i32(pinId), Nodes::PinShape_CircleFilled);
			auto* ns = access.TryGet<const CNamespace>(pinId);
			UI::Text(ns ? ns->name.ToString() : "none");
			Nodes::EndInput();

			Nodes::PopStyleColor(2);
			if (isInvalid)
			{
				Style::PopTextColor();
			}
		}
	}

	void DrawOutputs(
	    TAccessRef<CInvalid, CExprType, CNamespace> access, const CExprOutputs& outputs)
	{
		for (AST::Id pinId : outputs.pinIds)
		{
			if (!access.IsValid(pinId))
			{
				continue;
			}

			AST::Id typeId       = AST::NoId;
			Color pinColor       = Style::GetTypeColor<void>();
			const bool isInvalid = access.Has<CInvalid>(pinId);
			if (isInvalid)
			{
				Style::PushTextColor(Style::invalidColor);
				pinColor = Style::invalidColor;
			}
			else if (auto* type = access.TryGet<const CExprType>(pinId))
			{
				typeId = type->id;
				pinColor =
				    Style::GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), typeId);
			}
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(pinColor));

			Nodes::BeginOutput(i32(pinId), Nodes::PinShape_CircleFilled);
			auto* ns = access.TryGet<const CNamespace>(pinId);
			UI::Text(ns ? ns->name.ToString() : "none");
			Nodes::EndOutput();

			Nodes::PopStyleColor(2);
			if (isInvalid)
			{
				Style::PopTextColor();
			}
		}
	}

	void BeginNode(TAccessRef<TWrite<CNodePosition>> access, AST::Id id)
	{
		currentNodeTransform = &access.GetOrAdd<CNodePosition>(id);
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
			UI::SetNextItemWidth(math::Max(settings.GetGridSize() * 4.f, 30.f));
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
			UI::SetNextItemWidth(math::Max(settings.GetGridSize() * 4.f, 30.f));
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
			const v2 size{math::Max(minSize.x, textSize.x), math::Max(minSize.y, textSize.y)};
			UI::InputTextMultiline("##value", value, v2(size - settings.GetContentPadding()));
			PopInnerNodeStyle();
			Nodes::EndOutput();
		}
		EndNode(ast);

		Nodes::PopStyleColor(2);
		Style::PopNodeBackgroundColor();
	}

	using FunctionDeclsAccess = TAccessRef<CExprOutputs, CInvalid, CExprType, CNamespace,
	    TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef, TWrite<CNodePosition>>;
	void DrawFunctionDecls(FunctionDeclsAccess access, const TArray<AST::Id>& functionDecls)
	{
		for (AST::Id functionId : functionDecls)
		{
			Name name;
			if (auto* ns = access.TryGet<const CNamespace>(functionId))
			{
				name = ns->name;
			}

			Style::PushNodeBackgroundColor(rift::Style::GetNeutralColor(0));
			Style::PushNodeTitleColor(Style::functionColor);
			BeginNode(access, functionId);
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

				if (auto* outputs = access.TryGet<const CExprOutputs>(functionId))
				{
					DrawOutputs(access, *outputs);
				}
			}
			EndNode(access);
			Style::PopNodeTitleColor();
			Style::PopNodeBackgroundColor();
		}
	}

	void DrawReturnNode(
	    TAccessRef<TWrite<CNodePosition>, TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef>
	        access,
	    AST::Id id)
	{
		Style::PushNodeBackgroundColor(rift::Style::GetNeutralColor(0));
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

	using CallsAccess =
	    TAccessRef<TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef, CExprCall, CExprInputs,
	        CExprOutputs, CNamespace, CExprType, CInvalid, TWrite<CNodePosition>, CType>;
	void DrawCalls(CallsAccess access, AST::Id typeId, const TArray<AST::Id>& childrenIds)
	{
		for (AST::Id id : childrenIds)
		{
			if (auto* call = access.TryGet<const CExprCall>(id))
			{
				StringView functionName = call->function.Last().ToString();

				Style::PushNodeBackgroundColor(rift::Style::GetNeutralColor(0));
				Style::PushNodeTitleColor(Style::callColor);
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
						UI::TextUnformatted(functionName.data());
						UI::EndGroup();

						UI::SameLine();
						Nodes::BeginOutput(i32(id), Nodes::PinShape_QuadFilled);
						UI::TextUnformatted("");
						Nodes::EndOutput();

						PopExecutionPinStyle();
					}
					Nodes::EndNodeTitleBar();

					// Inputs
					UI::BeginGroup();
					if (const auto* inputs = access.TryGet<const CExprInputs>(id))
					{
						DrawInputs(access, *inputs);
					}
					UI::EndGroup();
					UI::SameLine();

					// Outputs
					UI::BeginGroup();
					if (const auto* outputs = access.TryGet<const CExprOutputs>(id))
					{
						DrawOutputs(access, *outputs);
					}
					UI::EndGroup();
				}
				EndNode(access);
				Style::PopNodeTitleColor();
				Style::PopNodeBackgroundColor();
			}
		}
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

	void DrawReturns(TAccessRef<TWrite<CNodePosition>, TWrite<CChanged>, TWrite<CFileDirty>, CChild,
	                     CFileRef, CStmtReturn>
	                     access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : GetIf<CStmtReturn>(access, children))
		{
			DrawReturnNode(access, id);
		}
	}

	void DrawLiterals(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		for (AST::Id id : ecs::GetIf<CLiteralBool>(ast, children))
		{
			DrawLiteralBool(ast, id, ast.Get<CLiteralBool>(id).value);
		}

		for (AST::Id id : ecs::GetIf<CLiteralIntegral>(ast, children))
		{
			DrawLiteralIntegral(ast, id, ast.Get<CLiteralIntegral>(id));
		}

		for (AST::Id id : ecs::GetIf<CLiteralFloating>(ast, children))
		{
			DrawLiteralFloating(ast, id, ast.Get<CLiteralFloating>(id));
		}

		for (AST::Id id : ecs::GetIf<CLiteralString>(ast, children))
		{
			DrawLiteralString(ast, id, ast.Get<CLiteralString>(id).value);
		}
	}

	void DrawVariableRefs(AST::Tree& ast, const TArray<AST::Id>& children)
	{
		String name;
		for (AST::Id id : ecs::GetIf<CExprDeclRefId>(ast, children))
		{
			AST::Id variableId = ast.Get<const CExprDeclRefId>(id).declarationId;

			const CExprType* exprType = ast.TryGet<CExprType>(id);
			AST::Id typeId            = exprType ? exprType->id : AST::NoId;

			const Color color = Style::GetTypeColor(ast, typeId);
			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(ast, id);
			{
				Nodes::BeginOutput(i32(id), Nodes::PinShape_CircleFilled);
				PushInnerNodeStyle();
				StringView name = "Invalid";
				if (ast.IsValid(variableId) && ast.Has<CNamespace>(variableId))
				{
					name = ast.Get<const CNamespace>(variableId).name.ToString();
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

	void DrawIfs(TAccessRef<TWrite<CNodePosition>, TWrite<CChanged>, TWrite<CFileDirty>, CChild,
	                 CFileRef, CStmtIf, CStmtOutputs, CExprInputs, CParent>
	                 access,
	    const TArray<AST::Id>& children)
	{
		Style::PushNodeBackgroundColor(Style::GetNeutralColor(0));
		Style::PushNodeTitleColor(Style::flowColor);
		for (AST::Id id : ecs::GetIf<CStmtIf, CExprInputs, CStmtOutputs>(access, children))
		{
			BeginNode(access, id);
			{
				Nodes::BeginNodeTitleBar();
				{
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

						auto& inputs = access.Get<const CExprInputs>(id);
						if (!Ensure(inputs.pinIds.Size() == 1))
						{
							continue;
						}
						Nodes::BeginInput(i32(inputs.pinIds[0]), Nodes::PinShape_CircleFilled);
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
						auto& outputs = access.Get<const CStmtOutputs>(id);
						if (!Ensure(outputs.pinIds.Size() == 2))
						{
							continue;
						}
						PushExecutionPinStyle();
						Nodes::BeginOutput(i32(outputs.pinIds[0]), Nodes::PinShape_QuadFilled);
						UI::TextUnformatted("true");
						Nodes::EndOutput();
						Nodes::BeginOutput(i32(outputs.pinIds[1]), Nodes::PinShape_QuadFilled);
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

	void DrawUnaryOperators(TAccessRef<TWrite<CNodePosition>, TWrite<CChanged>, TWrite<CFileDirty>,
	                            CChild, CParent, CFileRef, CExprUnaryOperator>
	                            access,
	    const TArray<AST::Id>& children)
	{
		for (AST::Id id : ecs::GetIf<CExprUnaryOperator>(access, children))
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
				StringView shortName = Types::GetUnaryOperatorName(op.type);
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

	void DrawBinaryOperators(TAccessRef<TWrite<CNodePosition>, TWrite<CChanged>, TWrite<CFileDirty>,
	                             CChild, CParent, CFileRef, CExprBinaryOperator>
	                             access,
	    const TArray<AST::Id>& children)
	{
		TArray<AST::Id> pinIds;
		for (AST::Id id : ecs::GetIf<CExprBinaryOperator>(access, children))
		{
			static constexpr Color color = Style::GetNeutralColor(0);

			Style::PushNodeBackgroundColor(color);
			Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
			Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, Style::Hovered(color));

			BeginNode(access, id);
			{
				pinIds.Clear(false);
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
				StringView shortName = Types::GetBinaryOperatorName(op.type);
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
	    TAccessRef<CParent, CStmtOutput, CStmtOutputs>& access, const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 2.f);
		Nodes::PushStyleColor(Nodes::ColorVar_Link, Style::executionColor);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(Style::executionColor));
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		for (AST::Id outputId : GetIf<CStmtOutput>(access, children))
		{
			const auto* output = access.TryGet<const CStmtOutput>(outputId);
			if (output && access.IsValid(output->linkInputNode))
			{
				// Input pin ids equal input node ids
				// Output pin ids equal output node ids
				Nodes::Link(i32(output->linkInputNode), i32(outputId), i32(output->linkInputNode));
			}
		}

		for (AST::Id outputId : GetIf<CStmtOutputs>(access, children))
		{
			if (const auto* outputs = access.TryGet<const CStmtOutputs>(outputId))
			{
				if (EnsureMsg(outputs->linkInputNodes.Size() == outputs->pinIds.Size(),
				        "Inputs and pins must match. Graph might be corrupted."))
				{
					for (i32 i = 0; i < outputs->linkInputNodes.Size(); ++i)
					{
						const AST::Id outputPinId = outputs->pinIds[i];
						const AST::Id inputNodeId = outputs->linkInputNodes[i];
						if (access.IsValid(outputPinId) && access.IsValid(inputNodeId))
						{
							// Input pin ids equal input node ids
							// TODO: Execution pin ids atm are the same as the node id.
							// Implement proper output pin support
							Nodes::Link(i32(inputNodeId), i32(outputPinId), i32(inputNodeId));
						}
					}
				}
			}
		}
		Nodes::PopStyleColor(3);
		Nodes::PopStyleVar();
	}

	void DrawExpressionLinks(TAccessRef<CParent, CExprInputs, CExprType, CInvalid>& access,
	    const TArray<AST::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 1.5f);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, Style::selectedColor);

		for (AST::Id nodeId : ecs::GetIf<CExprInputs>(access, children))
		{
			const auto& inputs = access.Get<const CExprInputs>(nodeId);
			if (!EnsureMsg(inputs.pinIds.Size() == inputs.linkedOutputs.Size(),
			        "Inputs are invalid. The graph might be corrupted.")) [[likely]]
			{
				continue;
			}

			for (i32 i = 0; i < inputs.linkedOutputs.Size(); ++i)
			{
				AST::Id inputId = inputs.pinIds[i];
				OutputId output = inputs.linkedOutputs[i];
				if (!access.IsValid(inputId) || !access.IsValid(output.pinId))
				{
					continue;
				}

				Color color = Style::GetTypeColor<void>();
				if (access.Has<CInvalid>(inputId) || access.Has<CInvalid>(output.pinId))
				{
					color = Style::invalidColor;
				}
				else if (const auto* type = access.TryGet<const CExprType>(output.pinId))
				{
					color =
					    Style::GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), type->id);
				}
				else if (const auto* type = access.TryGet<const CExprType>(inputId))
				{
					color =
					    Style::GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), type->id);
				}

				Nodes::PushStyleColor(Nodes::ColorVar_Link, color);
				Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, Style::Hovered(color));

				Nodes::Link(i32(inputId), i32(output.pinId), i32(inputId));

				Nodes::PopStyleColor(2);
			}
		}
		Nodes::PopStyleColor();
		Nodes::PopStyleVar();
	}

	void DrawTypeGraph(AST::Tree& ast, AST::Id typeId, CTypeEditor& typeEditor)
	{
		if (!typeEditor.showGraph)
		{
			return;
		}

		static String graphId;
		graphId.clear();
		Strings::FormatTo(graphId, "Graph##{}", typeId);

		bool wantsToOpenContextMenu = false;
		typeEditor.layout.BindNextWindowToNode(CTypeEditor::centralNode, ImGuiCond_Appearing);
		if (UI::Begin(graphId.c_str(), &typeEditor.showGraph, ImGuiWindowFlags_NoCollapse))
		{
			Nodes::SetEditorContext(&typeEditor.nodesEditor);
			Nodes::GetCurrentContext()->canCreateLinks = Types::CanEditFunctionBodies(ast, typeId);
			Nodes::BeginNodeEditor();
			PushNodeStyle();

			if (!ImGui::IsAnyItemHovered() && Nodes::IsEditorHovered()
			    && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				wantsToOpenContextMenu = true;
			}

			TArray<AST::Id> children;
			AST::Hierarchy::GetChildren(ast, typeId, children);

			// Nodes
			DrawFunctionDecls(ast, ecs::GetIf<CDeclFunction>(ast, children));
			DrawReturns(ast, children);
			DrawCalls(ast, typeId, children);
			DrawLiterals(ast, children);
			DrawVariableRefs(ast, children);

			DrawIfs(ast, children);
			DrawUnaryOperators(ast, children);
			DrawBinaryOperators(ast, children);

			// Links
			DrawStatementLinks(ast, children);
			DrawExpressionLinks(ast, children);

			Nodes::DrawMiniMap(0.2f, Nodes::MiniMapCorner::TopRight);
			PopNodeStyle();

			if (UI::IsKeyReleased(GLFW_KEY_DELETE))
			{
				Types::RemoveNodes(ast, Nodes::GetSelectedNodes());
			}
			Nodes::EndNodeEditor();

			Nodes::Id outputPin;
			Nodes::Id inputPin;
			if (Nodes::IsLinkCreated(outputPin, inputPin))
			{
				AST::Statements::TryConnect(ast, AST::Id(outputPin), AST::Id(inputPin));
				AST::Expressions::TryConnect(ast,
				    AST::Expressions::OutputFromPinId(ast, AST::Id(outputPin)),
				    AST::Expressions::InputFromPinId(ast, AST::Id(inputPin)));
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				// linkId is always the outputId
				AST::Statements::Disconnect(ast, AST::Id(linkId));
				AST::Expressions::Disconnect(
				    ast, AST::Expressions::InputFromPinId(ast, AST::Id(linkId)));
			}

			AST::Id hoveredNodeId = Nodes::GetHoveredNode();
			if (!IsNone(hoveredNodeId) && Nodes::IsNodeSelected(hoveredNodeId)
			    && UI::IsMouseClicked(ImGuiMouseButton_Left))
			{
				typeEditor.selectedPropertyId = Nodes::GetHoveredNode();
			}

			static AST::Id contextHoveredNodeId = AST::NoId;
			static AST::Id contextHoveredLinkId = AST::NoId;
			if (wantsToOpenContextMenu)
			{
				contextHoveredNodeId = Nodes::GetHoveredNode();
				contextHoveredLinkId = Nodes::GetHoveredLink();
				ImGui::OpenPopup("ContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast, typeId, contextHoveredNodeId, contextHoveredLinkId);
		}
		UI::End();
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
}    // namespace rift::Graph
