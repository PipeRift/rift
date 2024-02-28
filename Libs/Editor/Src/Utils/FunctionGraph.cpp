// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Components/Literals.h>
#include <AST/Components/Statements.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <PipeECS.h>
#include <UI/Style.h>
#include <Utils/Nodes.h>
#include <Utils/NodesInternal.h>
#include <Utils/NodesMiniMap.h>


namespace rift::editor::Graph
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

	void BeginExprInput(p::TAccessRef<ast::CExprTypeId> access, ast::Id id, const bool& invalid)
	{
		bool isPointer = false;
		ast::Id typeId = ast::NoId;
		if (auto* type = access.TryGet<const ast::CExprTypeId>(id))
		{
			typeId    = type->id;
			isPointer = type->mode != ast::TypeMode::Value;
		}

		Color pinColor = GetTypeColor<void>();
		if (invalid)
		{
			UI::PushTextColor(invalidColor);
			pinColor = invalidColor;
		}
		else
		{
			pinColor = GetTypeColor(static_cast<ast::Tree&>(access.GetContext()), typeId);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::Hovered(pinColor));

		Nodes::BeginInput(
		    i32(id), isPointer ? Nodes::PinShape_DiamondFilled : Nodes::PinShape_CircleFilled);
	}

	void BeginExprOutput(p::TAccessRef<ast::CExprTypeId> access, ast::Id id, const bool& invalid)
	{
		bool isPointer = false;
		ast::Id typeId = ast::NoId;
		if (auto* type = access.TryGet<const ast::CExprTypeId>(id))
		{
			typeId    = type->id;
			isPointer = type->mode != ast::TypeMode::Value;
		}

		Color pinColor = GetTypeColor<void>();
		if (invalid)
		{
			UI::PushTextColor(invalidColor);
			pinColor = invalidColor;
		}
		else
		{
			pinColor = GetTypeColor(static_cast<ast::Tree&>(access.GetContext()), typeId);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::Hovered(pinColor));

		Nodes::BeginOutput(
		    i32(id), isPointer ? Nodes::PinShape_DiamondFilled : Nodes::PinShape_CircleFilled);
	}

	void EndExprInput(const bool& invalid)
	{
		Nodes::EndInput();
		Nodes::PopStyleColor(2);
		if (invalid)
		{
			UI::PopTextColor();
		}
	}

	void EndExprOutput(const bool& invalid)
	{
		Nodes::EndOutput();
		Nodes::PopStyleColor(2);
		if (invalid)
		{
			UI::PopTextColor();
		}
	}

	void DrawInputs(p::TAccessRef<ast::CInvalid, ast::CExprTypeId, ast::CNamespace> access,
	    const ast::CExprInputs& inputs)
	{
		for (ast::Id pinId : inputs.pinIds)
		{
			if (access.IsValid(pinId))
			{
				const bool invalid = access.Has<ast::CInvalid>(pinId);
				BeginExprInput(access, pinId, invalid);
				auto* ns = access.TryGet<const ast::CNamespace>(pinId);
				UI::Text(ns ? ns->name.AsString() : "none");
				EndExprInput(invalid);
			}
		}
	}

	void DrawOutputs(p::TAccessRef<ast::CInvalid, ast::CExprTypeId, ast::CNamespace> access,
	    const ast::CExprOutputs& outputs)
	{
		for (ast::Id pinId : outputs.pinIds)
		{
			if (access.IsValid(pinId))
			{
				const bool invalid = access.Has<ast::CInvalid>(pinId);
				BeginExprOutput(access, pinId, invalid);
				auto* ns = access.TryGet<const ast::CNamespace>(pinId);
				UI::Text(ns ? ns->name.AsString() : "none");
				EndExprOutput(invalid);
			}
		}
	}

	void BeginNode(p::TAccessRef<TWrite<CNodePosition>> access, ast::Id id)
	{
		currentNodeTransform = &access.GetOrAdd<CNodePosition>(id);
		auto* context        = Nodes::GetCurrentContext();
		if (UI::IsWindowAppearing()
		    || (!context->leftMouseDragging && !context->leftMouseReleased
		        && !Nodes::IsNodeSelected(id)))
		{
			SetNodePosition(id, currentNodeTransform->position);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_NodeOutline, selectedColor);
		Nodes::BeginNode(id);
	}

	void EndNode(const ast::TransactionAccess& access)
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
			const ast::Id id = context->currentNodeId;
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
		static constexpr Color color = executionColor;
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::Hovered(color));
	}

	void PopExecutionPinStyle()
	{
		Nodes::PopStyleColor(2);
	}

	void DrawLiteralBool(ast::Tree& ast, ast::Id id, bool& value)
	{
		static constexpr Color color = GetTypeColor<bool>();

		PushNodeBackgroundColor(color);

		BeginNode(ast, id);
		{
			BeginExprOutput(ast, id, false);
			PushInnerNodeStyle();
			UI::Checkbox("##value", &value);
			PopInnerNodeStyle();
			EndExprOutput(false);
		}
		EndNode(ast);
		PopNodeBackgroundColor();
	}

	void DrawLiteralIntegral(ast::Tree& ast, ast::Id id, ast::CLiteralIntegral& value)
	{
		const bool isSigned = value.IsSigned();
		const Color color   = isSigned ? GetTypeColor<i32>() : GetTypeColor<u32>();
		PushNodeBackgroundColor(color);

		BeginNode(ast, id);
		{
			BeginExprOutput(ast, id, false);
			PushInnerNodeStyle();

			ImGuiDataType dataType = ImGuiDataType_COUNT;
			switch (value.type)
			{
				case ast::IntegralType::S8: dataType = ImGuiDataType_S8; break;
				case ast::IntegralType::S16: dataType = ImGuiDataType_S16; break;
				case ast::IntegralType::S32: dataType = ImGuiDataType_S32; break;
				case ast::IntegralType::S64: dataType = ImGuiDataType_S64; break;
				case ast::IntegralType::U8: dataType = ImGuiDataType_U8; break;
				case ast::IntegralType::U16: dataType = ImGuiDataType_U16; break;
				case ast::IntegralType::U32: dataType = ImGuiDataType_U32; break;
				case ast::IntegralType::U64: dataType = ImGuiDataType_U64; break;
			}
			const char* format = value.IsSigned() ? "%i" : "%iu";

			char buf[64];
			UI::DataTypeFormatString(buf, IM_ARRAYSIZE(buf), dataType, &value.value, format);
			UI::SetNextItemWidth(5.f + p::Max(UI::CalcTextSize(buf).x, 20.f));
			UI::InputScalar("##value", dataType, &value.value, nullptr, nullptr, format);

			PopInnerNodeStyle();
			EndExprOutput(false);
		}
		EndNode(ast);
		PopNodeBackgroundColor();
	}

	void DrawLiteralFloating(ast::Tree& ast, ast::Id id, ast::CLiteralFloating& value)
	{
		const bool isDouble = value.type == ast::FloatingType::F64;
		const Color color   = isDouble ? GetTypeColor<double>() : GetTypeColor<float>();
		PushNodeBackgroundColor(color);
		BeginNode(ast, id);
		{
			BeginExprOutput(ast, id, false);
			PushInnerNodeStyle();

			const ImGuiDataType dataType = isDouble ? ImGuiDataType_Double : ImGuiDataType_Float;
			char buf[64];
			UI::DataTypeFormatString(buf, IM_ARRAYSIZE(buf), dataType, &value.value, "%.15g");
			UI::SetNextItemWidth(5.f + p::Max(UI::CalcTextSize(buf).x, 20.f));
			UI::InputScalar("##value", dataType, &value.value, nullptr, nullptr, "%.15g");

			PopInnerNodeStyle();
			EndExprOutput(false);
		}
		EndNode(ast);
		PopNodeBackgroundColor();
	}

	void DrawLiteralString(ast::Tree& ast, ast::Id id, String& value)
	{
		static constexpr Color color = GetTypeColor<String>();
		PushNodeBackgroundColor(color);

		BeginNode(ast, id);
		{
			BeginExprOutput(ast, id, false);
			PushInnerNodeStyle();
			ImGuiStyle& style     = ImGui::GetStyle();
			const ImVec2 textSize = ImGui::CalcTextSize(value.data(), value.data() + value.size());
			const v2 minSize{settings.GetGridSize() * 4.f, settings.GetGridSize()};
			const v2 size{p::Max(minSize.x, textSize.x), p::Max(minSize.y, textSize.y)};
			UI::InputTextMultiline("##value", value, v2(size - settings.GetContentPadding()));
			PopInnerNodeStyle();
			EndExprOutput(false);
		}
		EndNode(ast);

		PopNodeBackgroundColor();
	}

	using FunctionDeclsAccess = p::TAccessRef<ast::CExprOutputs, ast::CInvalid, ast::CExprTypeId,
	    ast::CNamespace, p::TWrite<ast::CChanged>, p::TWrite<ast::CFileDirty>, ast::CChild,
	    ast::CFileRef, p::TWrite<CNodePosition>>;
	void DrawFunctionDecls(FunctionDeclsAccess access, const TArray<ast::Id>& functionDecls)
	{
		for (ast::Id functionId : functionDecls)
		{
			Tag name;
			if (auto* ns = access.TryGet<const ast::CNamespace>(functionId))
			{
				name = ns->name;
			}

			PushNodeBackgroundColor(UI::GetNeutralColor(0));
			PushNodeTitleColor(functionColor);
			BeginNode(access, functionId);
			{
				Nodes::BeginNodeTitleBar();
				{
					UI::Text(name.AsString());
					UI::SameLine();

					PushExecutionPinStyle();
					Nodes::BeginOutput(i32(functionId), Nodes::PinShape_QuadFilled);
					UI::Text("");
					Nodes::EndOutput();
					PopExecutionPinStyle();
				}
				Nodes::EndNodeTitleBar();

				if (auto* outputs = access.TryGet<const ast::CExprOutputs>(functionId))
				{
					DrawOutputs(access, *outputs);
				}
			}
			EndNode(access);
			PopNodeTitleColor();
			PopNodeBackgroundColor();
		}
	}

	void DrawReturnNode(p::TAccessRef<TWrite<CNodePosition>, TWrite<ast::CChanged>,
	                        TWrite<ast::CFileDirty>, ast::CChild, ast::CFileRef>
	                        access,
	    ast::Id id)
	{
		PushNodeBackgroundColor(rift::UI::GetNeutralColor(0));
		PushNodeTitleColor(returnColor);
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
		PopNodeTitleColor();
		PopNodeBackgroundColor();
	}

	using CallsAccess = TAccessRef<TWrite<ast::CChanged>, TWrite<ast::CFileDirty>, ast::CChild,
	    ast::CFileRef, ast::CExprCall, ast::CExprInputs, ast::CExprOutputs, ast::CNamespace,
	    ast::CExprTypeId, ast::CInvalid, TWrite<CNodePosition>, ast::CDeclType>;
	void DrawCalls(CallsAccess access, ast::Id typeId, const TArray<ast::Id>& childrenIds)
	{
		for (ast::Id id : childrenIds)
		{
			if (auto* call = access.TryGet<const ast::CExprCall>(id))
			{
				StringView functionName = call->function.Last().AsString();

				PushNodeBackgroundColor(rift::UI::GetNeutralColor(0));
				PushNodeTitleColor(callColor);
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
					if (const auto* inputs = access.TryGet<const ast::CExprInputs>(id))
					{
						DrawInputs(access, *inputs);
					}
					UI::EndGroup();
					UI::SameLine();

					// Outputs
					UI::BeginGroup();
					if (const auto* outputs = access.TryGet<const ast::CExprOutputs>(id))
					{
						DrawOutputs(access, *outputs);
					}
					UI::EndGroup();
				}
				EndNode(access);
				PopNodeTitleColor();
				PopNodeBackgroundColor();
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
		Nodes::PushStyleVar(Nodes::StyleVar_PinDiamondSideLength, 10.f);
		Nodes::PushStyleVar(Nodes::StyleVar_NodePadding, v2(10.f, 2.f));

		Nodes::PushStyleVar(Nodes::StyleVar_NodeBorderThickness, 0.f);

		// UI::PushStyleCompact();
	}

	void PopNodeStyle()
	{
		// UI::PopStyleCompact();
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

	void DrawReturns(p::TAccessRef<TWrite<CNodePosition>, TWrite<ast::CChanged>,
	                     TWrite<ast::CFileDirty>, ast::CChild, ast::CFileRef, ast::CStmtReturn>
	                     access,
	    const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CStmtReturn>(access, children))
		{
			DrawReturnNode(access, id);
		}
	}

	void DrawLiterals(ast::Tree& ast, const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CLiteralBool>(ast, children))
		{
			DrawLiteralBool(ast, id, ast.Get<ast::CLiteralBool>(id).value);
		}

		for (ast::Id id : FindIdsWith<ast::CLiteralIntegral>(ast, children))
		{
			DrawLiteralIntegral(ast, id, ast.Get<ast::CLiteralIntegral>(id));
		}

		for (ast::Id id : FindIdsWith<ast::CLiteralFloating>(ast, children))
		{
			DrawLiteralFloating(ast, id, ast.Get<ast::CLiteralFloating>(id));
		}

		for (ast::Id id : FindIdsWith<ast::CLiteralString>(ast, children))
		{
			DrawLiteralString(ast, id, ast.Get<ast::CLiteralString>(id).value);
		}
	}

	void DrawVariableRefs(ast::Tree& ast, const TArray<ast::Id>& children)
	{
		String name;
		for (ast::Id id : FindIdsWith<ast::CExprDeclRefId>(ast, children))
		{
			ast::Id variableId = ast.Get<const ast::CExprDeclRefId>(id).declarationId;

			const ast::CExprTypeId* exprType = ast.TryGet<ast::CExprTypeId>(id);
			ast::Id typeId                   = exprType ? exprType->id : ast::NoId;

			const Color color = GetTypeColor(ast, typeId);
			PushNodeBackgroundColor(color);

			BeginNode(ast, id);
			{
				BeginExprOutput(ast, id, false);
				PushInnerNodeStyle();
				StringView name = "Invalid";
				if (ast.IsValid(variableId) && ast.Has<ast::CNamespace>(variableId))
				{
					name = ast.Get<const ast::CNamespace>(variableId).name.AsString();
				}
				UI::Text(name);
				PopInnerNodeStyle();
				EndExprOutput(false);
			}
			EndNode(ast);

			PopNodeBackgroundColor();
		}
	}

	void DrawIfs(p::TAccessRef<TWrite<CNodePosition>, TWrite<ast::CChanged>,
	                 TWrite<ast::CFileDirty>, ast::CChild, ast::CFileRef, ast::CStmtIf,
	                 ast::CStmtOutputs, ast::CExprInputs, ast::CParent, ast::CExprTypeId>
	                 access,
	    const TArray<ast::Id>& children)
	{
		PushNodeBackgroundColor(UI::GetNeutralColor(0));
		PushNodeTitleColor(flowColor);
		for (ast::Id id :
		    FindIdsWith<ast::CStmtIf, ast::CExprInputs, ast::CStmtOutputs>(access, children))
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

						auto& inputs = access.Get<const ast::CExprInputs>(id);
						if (!Ensure(inputs.pinIds.Size() == 1))
						{
							continue;
						}
						BeginExprInput(access, inputs.pinIds[0], false);
						UI::TextUnformatted("");
						EndExprInput(false);
					}
					UI::EndGroup();
					UI::SameLine();

					UI::BeginGroup();
					UI::TextUnformatted("if");
					UI::EndGroup();

					UI::SameLine();
					UI::BeginGroup();
					{
						auto& outputs = access.Get<const ast::CStmtOutputs>(id);
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
		PopNodeTitleColor();
		PopNodeBackgroundColor();
	}

	void DrawUnaryOperators(
	    TAccessRef<TWrite<CNodePosition>, TWrite<ast::CChanged>, TWrite<ast::CFileDirty>,
	        ast::CChild, ast::CParent, ast::CFileRef, ast::CExprUnaryOperator, ast::CExprTypeId>
	        access,
	    const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CExprUnaryOperator>(access, children))
		{
			static constexpr Color color = UI::GetNeutralColor(0);

			PushNodeBackgroundColor(color);

			BeginNode(access, id);
			{
				BeginExprInput(access, id, false);
				UI::TextUnformatted("");
				EndExprInput(false);
				UI::SameLine();

				const auto& op       = access.Get<const ast::CExprUnaryOperator>(id);
				StringView shortName = editor::GetUnaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				BeginExprOutput(access, id, false);
				UI::TextUnformatted("");
				EndExprOutput(false);
			}
			EndNode(access);
			PopNodeBackgroundColor();
		}
	}

	void DrawBinaryOperators(
	    TAccessRef<TWrite<CNodePosition>, TWrite<ast::CChanged>, TWrite<ast::CFileDirty>,
	        ast::CChild, ast::CParent, ast::CFileRef, ast::CExprBinaryOperator, ast::CExprTypeId>
	        access,
	    const TArray<ast::Id>& children)
	{
		TArray<ast::Id> pinIds;
		for (ast::Id id : FindIdsWith<ast::CExprBinaryOperator>(access, children))
		{
			static constexpr Color color = UI::GetNeutralColor(0);
			PushNodeBackgroundColor(color);

			BeginNode(access, id);
			{
				pinIds.Clear(false);
				p::GetIdChildren(access, id, pinIds);
				if (!Ensure(pinIds.Size() >= 2))
				{
					continue;
				}
				UI::BeginGroup();
				BeginExprInput(access, pinIds[0], false);
				UI::TextUnformatted("");
				EndExprInput(false);

				BeginExprInput(access, pinIds[1], false);
				UI::TextUnformatted("");
				EndExprInput(false);
				UI::EndGroup();
				UI::SameLine();

				const auto& op       = access.Get<const ast::CExprBinaryOperator>(id);
				StringView shortName = editor::GetBinaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				BeginExprOutput(access, id, false);
				UI::TextUnformatted("");
				EndExprOutput(false);
			}
			EndNode(access);
			PopNodeBackgroundColor();
		}
	}

	void DrawStatementLinks(
	    p::TAccessRef<ast::CParent, ast::CStmtOutput, ast::CStmtOutputs>& access,
	    const TArray<ast::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 2.f);
		Nodes::PushStyleColor(Nodes::ColorVar_Link, executionColor);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, UI::Hovered(executionColor));
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, selectedColor);

		for (ast::Id outputId : FindIdsWith<ast::CStmtOutput>(access, children))
		{
			const auto* output = access.TryGet<const ast::CStmtOutput>(outputId);
			if (output && access.IsValid(output->linkInputNode))
			{
				// Input pin ids equal input node ids
				// Output pin ids equal output node ids
				Nodes::Link(i32(output->linkInputNode), i32(outputId), i32(output->linkInputNode));
			}
		}

		for (ast::Id outputId : FindIdsWith<ast::CStmtOutputs>(access, children))
		{
			if (const auto* outputs = access.TryGet<const ast::CStmtOutputs>(outputId))
			{
				if (EnsureMsg(outputs->linkInputNodes.Size() == outputs->pinIds.Size(),
				        "Inputs and pins must match. Graph might be corrupted."))
				{
					for (i32 i = 0; i < outputs->linkInputNodes.Size(); ++i)
					{
						const ast::Id outputPinId = outputs->pinIds[i];
						const ast::Id inputNodeId = outputs->linkInputNodes[i];
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

	void DrawExpressionLinks(
	    TAccessRef<ast::CParent, ast::CExprInputs, ast::CExprTypeId, ast::CInvalid>& access,
	    const TArray<ast::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 1.5f);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, selectedColor);

		for (ast::Id nodeId : FindIdsWith<ast::CExprInputs>(access, children))
		{
			const auto& inputs = access.Get<const ast::CExprInputs>(nodeId);
			if (!EnsureMsg(inputs.pinIds.Size() == inputs.linkedOutputs.Size(),
			        "Inputs are invalid. The graph might be corrupted.")) [[likely]]
			{
				continue;
			}

			for (i32 i = 0; i < inputs.linkedOutputs.Size(); ++i)
			{
				ast::Id inputId        = inputs.pinIds[i];
				ast::ExprOutput output = inputs.linkedOutputs[i];
				if (!access.IsValid(inputId) || !access.IsValid(output.pinId))
				{
					continue;
				}

				Color color = GetTypeColor<void>();
				if (access.Has<ast::CInvalid>(inputId) || access.Has<ast::CInvalid>(output.pinId))
				{
					color = invalidColor;
				}
				else if (const auto* type = access.TryGet<const ast::CExprTypeId>(output.pinId))
				{
					color = GetTypeColor(static_cast<ast::Tree&>(access.GetContext()), type->id);
				}
				else if (const auto* type = access.TryGet<const ast::CExprTypeId>(inputId))
				{
					color = GetTypeColor(static_cast<ast::Tree&>(access.GetContext()), type->id);
				}

				Nodes::PushStyleColor(Nodes::ColorVar_Link, color);
				Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, UI::Hovered(color));

				Nodes::Link(i32(inputId), i32(output.pinId), i32(inputId));

				Nodes::PopStyleColor(2);
			}
		}
		Nodes::PopStyleColor();
		Nodes::PopStyleVar();
	}

	void DrawTypeGraph(ast::Tree& ast, ast::Id typeId, CTypeEditor& typeEditor)
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
			Nodes::GetCurrentContext()->canCreateLinks = ast::HasFunctionBodies(ast, typeId);
			Nodes::BeginNodeEditor();
			PushNodeStyle();

			if (!ImGui::IsAnyItemHovered() && Nodes::IsEditorHovered()
			    && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				wantsToOpenContextMenu = true;
			}

			TArray<ast::Id> children;
			p::GetIdChildren(ast, typeId, children);

			// Nodes
			DrawFunctionDecls(ast, FindIdsWith<ast::CDeclFunction>(ast, children));
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

			if (UI::IsKeyReleased(ImGuiKey_Delete))
			{
				ast::RemoveNodes(ast, Nodes::GetSelectedNodes());
			}
			Nodes::EndNodeEditor();


			Nodes::Id outputPin;
			Nodes::Id inputPin;
			if (Nodes::IsLinkCreated(outputPin, inputPin))
			{
				ast::Id pinIds[2]{ast::Id(outputPin), ast::Id(inputPin)};
				ScopedChange(ast, pinIds);
				ast::TryConnectStmt(ast, ast::Id(outputPin), ast::Id(inputPin));
				ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, ast::Id(outputPin)),
				    ast::GetExprInputFromPin(ast, ast::Id(inputPin)));
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				ScopedChange(ast, ast::Id(linkId));
				// linkId is always the outputId
				ast::DisconnectStmtLink(ast, ast::Id(linkId));
				ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, ast::Id(linkId)));
			}

			ast::Id hoveredNodeId = Nodes::GetHoveredNode();
			if (!IsNone(hoveredNodeId) && Nodes::IsNodeSelected(hoveredNodeId)
			    && UI::IsMouseClicked(ImGuiMouseButton_Left))
			{
				typeEditor.selectedPropertyId = Nodes::GetHoveredNode();
			}

			static ast::Id contextHoveredNodeId = ast::NoId;
			static ast::Id contextHoveredLinkId = ast::NoId;
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

	void SetNodePosition(ast::Id id, v2 position)
	{
		position *= settings.GetGridSize();
		Nodes::SetNodeGridSpacePos(id, position);
	}

	v2 GetNodePosition(ast::Id id)
	{
		const v2 pos = Nodes::GetNodeGridSpacePos(id);
		return v2{pos.x * settings.GetInvGridSize(), pos.y * settings.GetInvGridSize()}.Floor();
	}
}    // namespace rift::editor::Graph
