// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/FunctionGraph.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "Statics/EditorSettings.h"
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
#include <AST/Systems/CastSystem.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Settings.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/Log.h>
#include <PipeECS.h>
#include <UI/Style.h>
#include <Utils/Nodes.h>
#include <Utils/NodesInternal.h>
#include <Utils/NodesMiniMap.h>


namespace rift::editor::Graph
{
	static CNodePosition* currentNodeTransform = nullptr;

	// The pins of a cast node are only known once it has been drawn, so casts spawned from a
	// dropped link are placed on the next frame
	struct PendingCast
	{
		ast::Id castId;
		ast::Id outputPinId;
		ast::Id inputPinId;    // Pin of the node the cast feeds
	};
	static TArray<PendingCast> pendingCasts;

	// A value that only casts explicitly needs a cast node in between. Returns the new node,
	// or NoId when the link can't be fixed with a cast
	static ast::Id InsertCastNode(
	    ast::Tree& ast, ast::Id typeId, const ast::ExprOutput& output, const ast::ExprInput& input)
	{
		if (output.IsNone() || input.IsNone() || output.nodeId == input.nodeId
		    || ast::WouldExprLoop(ast, output.nodeId, input.nodeId))
		{
			return ast::NoId;
		}

		const ast::Id outputType = ast::GetPinType(ast, output.pinId);
		const ast::Id inputType  = ast::GetPinType(ast, input.pinId);
		if (p::IsNone(outputType) || p::IsNone(inputType)
		    || !ast::CastSystem::FindCast(ast, outputType, inputType))
		{
			return ast::NoId;
		}

		// The cast goes in between, with its input taking the source type and its output the
		// type of the pin it feeds
		const ast::Id castId    = ast::AddExplicitCast({ast, typeId}, outputType, inputType);
		const auto* castInputs  = ast.TryGet<const ast::CExprInputs>(castId);
		const auto* castOutputs = ast.TryGet<const ast::CExprOutputs>(castId);
		if (!castInputs || castInputs->pinIds.IsEmpty() || !castOutputs
		    || castOutputs->pinIds.IsEmpty())
		{
			return ast::NoId;
		}

		ast::TryConnectExpr(ast, output, ast::GetExprInputFromPin(ast, castInputs->pinIds[0]));
		ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, castOutputs->pinIds[0]), input);
		return castId;
	}

	namespace
	{
		// Grid tiles moved per key press while shift is held
		constexpr float fastMoveTiles = 5.f;

		// Persists the current zoom after it stops changing, so scrolling does not write the
		// settings file on every notch.
		void PersistZoom()
		{
			static float saveTimer = -1.f;
			static float lastSaved = 0.f;

			ZoomSettings& zoom = GetUserSettings<EditorSettings>().zoom;

			if (zoom.current != lastSaved)
			{
				saveTimer = 0.5f;
			}

			// Once the value settles, write it, even if the focus moved elsewhere meanwhile.
			if (saveTimer >= 0.f)
			{
				saveTimer -= ImGui::GetIO().DeltaTime;
				if (saveTimer < 0.f)
				{
					SaveUserSettings<EditorSettings>();
					lastSaved = zoom.current;
				}
			}
		}
	}    // namespace

	void ApplyGraphSettings()
	{
		const EditorSettings& editorSettings = GetUserSettings<EditorSettings>();

		// Grid size also publishes the spacing into the node style.
		settings.SetGridSize(p::Clamp(editorSettings.gridSize, 4.f, 64.f));
	}

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

	float Settings::GetSpaceHeight() const
	{
		return (gridSize - (verticalMargin * 2.f)) * Nodes::GetZoom();
	}

	v2 Settings::GetGridPosition(v2 screenPosition) const
	{
		return Nodes::ScreenToGridPosition(screenPosition) * GetInvGridSize();
	}

	void BeginExprInput(p::TIdScopeRef<ast::CExprTypeId> scope, ast::Id id, const bool& invalid)
	{
		bool isPointer = false;
		ast::Id typeId = ast::NoId;
		if (auto* type = scope.TryGet<const ast::CExprTypeId>(id))
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
			pinColor = GetTypeColor(static_cast<ast::Tree&>(scope.GetContext()), typeId);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::ToHovered(pinColor));
		Nodes::BeginInput(i32(id.value),
		    isPointer ? Nodes::PinShape_DiamondFilled : Nodes::PinShape_CircleFilled);
	}

	void BeginExprOutput(p::TIdScopeRef<ast::CExprTypeId> scope, ast::Id id, const bool& invalid)
	{
		bool isPointer = false;
		ast::Id typeId = ast::NoId;
		if (auto* type = scope.TryGet<const ast::CExprTypeId>(id))
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
			pinColor = GetTypeColor(static_cast<ast::Tree&>(scope.GetContext()), typeId);
		}

		Nodes::PushStyleColor(Nodes::ColorVar_Pin, pinColor);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::ToHovered(pinColor));
		Nodes::BeginOutput(i32(id.value),
		    isPointer ? Nodes::PinShape_DiamondFilled : Nodes::PinShape_CircleFilled);
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

	void DrawInputs(p::TIdScopeRef<ast::CInvalid, ast::CExprTypeId, ast::CNamespace> scope,
	    const ast::CExprInputs& inputs)
	{
		for (ast::Id pinId : inputs.pinIds)
		{
			if (scope.IsValid(pinId))
			{
				const bool invalid = scope.Has<ast::CInvalid>(pinId);
				BeginExprInput(scope, pinId, invalid);
				auto* ns = scope.TryGet<const ast::CNamespace>(pinId);
				UI::Text(ns ? ns->name.AsString() : "none");
				EndExprInput(invalid);
			}
		}
	}

	void DrawOutputs(p::TIdScopeRef<ast::CInvalid, ast::CExprTypeId, ast::CNamespace> scope,
	    const ast::CExprOutputs& outputs)
	{
		for (ast::Id pinId : outputs.pinIds)
		{
			if (scope.IsValid(pinId))
			{
				const bool invalid = scope.Has<ast::CInvalid>(pinId);
				BeginExprOutput(scope, pinId, invalid);
				auto* ns = scope.TryGet<const ast::CNamespace>(pinId);
				UI::Text(ns ? ns->name.AsString() : "none");
				EndExprOutput(invalid);
			}
		}
	}

	void BeginNode(p::TIdScopeRef<Writes<CNodePosition>> scope, ast::Id id)
	{
		currentNodeTransform = &scope.GetOrAdd<CNodePosition>(id);
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

	void EndNode(const ast::TransactionScope& scope)
	{
		// Selection outline
		const auto* context = Nodes::GetCurrentContext();
		if (Nodes::IsNodeSelected(context->currentNodeId))
		{
			Nodes::GetEditorContext().nodes[context->currentNodeId].LayoutStyle.BorderThickness =
			    2.f * Nodes::GetZoom();
		}
		Nodes::EndNode();
		Nodes::PopStyleColor(1);

		if (currentNodeTransform && context->leftMouseReleased)
		{
			const ast::Id id = context->currentNodeId;
			v2 newPosition   = GetNodePosition(id);
			if (!newPosition.Equals(currentNodeTransform->position, 0.1f))
			{
				ScopedChange(scope, id);
				currentNodeTransform->position = newPosition;
			}
		}
		currentNodeTransform = nullptr;
	}

	void PushExecutionPinStyle()
	{
		static constexpr Color color = executionColor;
		Nodes::PushStyleColor(Nodes::ColorVar_Pin, color);
		Nodes::PushStyleColor(Nodes::ColorVar_PinHovered, UI::ToHovered(color));
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

			const float zoom     = Nodes::GetZoom();
			const float gridSize = settings.GetGridSize() * zoom;
			const v2 padding     = Nodes::GetStyle().NodePadding * zoom;
			const float fontSize = p::Max(gridSize - 2.f * padding.y, 1.f);

			i32 lineCount = 1;
			for (p::sizet i = 0; i < value.size(); ++i)
			{
				if (value[i] == '\n')
				{
					++lineCount;
				}
			}

			// One line of text per grid tile: scale the font to the tile and size the field
			// to the tiles minus the node padding
			ImGui::PushFont(ImGui::GetFont(), fontSize);
			const ImGuiStyle& style = ImGui::GetStyle();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{style.FramePadding.x, 0.f});
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{style.ItemSpacing.x, 0.f});

			const ImVec2 textSize = ImGui::CalcTextSize(value.data(), value.data() + value.size());
			// Keep a minimum width: with an empty value the computed width is 0 and ImGui
			// would stretch the field to the whole available width, making the node huge
			const float textWidth =
			    p::Ceil((textSize.x + 2.f * padding.x) / gridSize) * gridSize - 2.f * padding.x;
			const v2 size{p::Max(textWidth, 128.f * zoom), lineCount * gridSize - 2.f * padding.y};

			UI::InputTextMultiline("##value", value, size);
			EndExprOutput(false);
		}
		EndNode(ast);

		// Keep the zero spacing active until the node group is closed so the node ends up
		// exactly as its content plus the node padding
		ImGui::PopStyleVar(2);
		ImGui::PopFont();

		PopNodeBackgroundColor();
	}

	using FunctionDeclsScope =
	    p::TIdScopeRef<p::Writes<ast::CChanged, ast::CFileDirty, CNodePosition>, ast::CExprOutputs,
	        ast::CInvalid, ast::CExprTypeId, ast::CNamespace, ast::CChild, ast::CFileRef>;
	void DrawFunctionDecls(FunctionDeclsScope scope, const TArray<ast::Id>& functionDecls)
	{
		for (ast::Id functionId : functionDecls)
		{
			Tag name;
			if (auto* ns = scope.TryGet<const ast::CNamespace>(functionId))
			{
				name = ns->name;
			}

			PushNodeBackgroundColor(UI::GetNeutralColor(0));
			PushNodeTitleColor(functionColor);
			BeginNode(scope, functionId);
			{
				Nodes::BeginNodeTitleBar();
				{
					UI::Text(name.AsString());
					UI::SameLine();

					PushExecutionPinStyle();
					Nodes::BeginOutput(i32(functionId.value), Nodes::PinShape_TriangleFilled);
					UI::Text("");
					Nodes::EndOutput();
					PopExecutionPinStyle();
				}
				Nodes::EndNodeTitleBar();

				if (auto* outputs = scope.TryGet<const ast::CExprOutputs>(functionId))
				{
					DrawOutputs(scope, *outputs);
				}
			}
			EndNode(scope);
			PopNodeTitleColor();
			PopNodeBackgroundColor();
		}
	}

	void DrawReturnNode(
	    p::TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>, ast::CChild,
	        ast::CFileRef, ast::CExprInputs, ast::CInvalid, ast::CExprTypeId, ast::CNamespace>
	        scope,
	    ast::Id id)
	{
		PushNodeBackgroundColor(rift::UI::GetNeutralColor(0));
		PushNodeTitleColor(returnColor);
		BeginNode(scope, id);
		{
			Nodes::BeginNodeTitleBar();
			{
				PushExecutionPinStyle();
				Nodes::BeginInput(i32(id.value), Nodes::PinShape_TriangleFilled);
				UI::TextUnformatted("");
				Nodes::EndInput();
				UI::SameLine();
				PopExecutionPinStyle();

				UI::BeginGroup();
				UI::TextUnformatted("Return");
				UI::EndGroup();
			}
			Nodes::EndNodeTitleBar();

			// Return arguments are populated from the connected function's outputs
			if (const auto* inputs = scope.TryGet<const ast::CExprInputs>(id))
			{
				DrawInputs(scope, *inputs);
			}
		}
		EndNode(scope);
		PopNodeTitleColor();
		PopNodeBackgroundColor();
	}

	using CallsScope = TIdScopeRef<Writes<ast::CChanged, ast::CFileDirty, CNodePosition>,
	    ast::CChild, ast::CFileRef, ast::CExprCall, ast::CExprInputs, ast::CExprOutputs,
	    ast::CNamespace, ast::CExprTypeId, ast::CInvalid, ast::CDeclType>;
	void DrawCalls(CallsScope scope, ast::Id typeId, const TArray<ast::Id>& childrenIds)
	{
		for (ast::Id id : childrenIds)
		{
			if (auto* call = scope.TryGet<const ast::CExprCall>(id))
			{
				StringView functionName = call->function.Last().AsString();

				PushNodeBackgroundColor(rift::UI::GetNeutralColor(0));
				PushNodeTitleColor(callColor);
				BeginNode(scope, id);
				{
					Nodes::BeginNodeTitleBar();
					{
						PushExecutionPinStyle();

						Nodes::BeginInput(i32(id.value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("");
						Nodes::EndInput();
						UI::SameLine();

						UI::BeginGroup();
						UI::TextUnformatted(functionName.data());
						UI::EndGroup();

						UI::SameLine();
						Nodes::BeginOutput(i32(id.value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("");
						Nodes::EndOutput();

						PopExecutionPinStyle();
					}
					Nodes::EndNodeTitleBar();

					// Inputs
					UI::BeginGroup();
					if (const auto* inputs = scope.TryGet<const ast::CExprInputs>(id))
					{
						DrawInputs(scope, *inputs);
					}
					UI::EndGroup();
					UI::SameLine();

					// Outputs
					UI::BeginGroup();
					if (const auto* outputs = scope.TryGet<const ast::CExprOutputs>(id))
					{
						DrawOutputs(scope, *outputs);
					}
					UI::EndGroup();
				}
				EndNode(scope);
				PopNodeTitleColor();
				PopNodeBackgroundColor();
			}
		}
	}


	void Init()
	{
		Nodes::CreateContext();
		ApplyGraphSettings();
	}

	void Shutdown()
	{
		Nodes::DestroyContext();
		pendingCasts.Clear();
	}


	void PushNodeStyle()
	{
		Nodes::GetStyle().Flags |= Nodes::StyleFlags_GridLines | Nodes::StyleFlags_GridLinesPrimary
		                         | Nodes::StyleFlags_GridSnappingOnRelease;

		Nodes::PushStyleVar(Nodes::StyleVar_PinLineThickness, 2.5f);
		Nodes::PushStyleVar(Nodes::StyleVar_NodeCornerRounding, 1.f);
		Nodes::PushStyleVar(Nodes::StyleVar_PinTriangleSideLength, 10.f);
		Nodes::PushStyleVar(Nodes::StyleVar_PinDiamondSideLength, 10.f);
		Nodes::PushStyleVar(Nodes::StyleVar_NodePadding, v2(8.f, 1.f));
		Nodes::PushStyleVar(Nodes::StyleVar_NodeBorderThickness, 0.f);
	}

	void PopNodeStyle()
	{
		Nodes::PopStyleVar(6);
	}

	void PushInnerNodeStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const float padding =
		    (settings.GetSpaceHeight() - GImGui->FontSize) * 0.5f - settings.verticalPadding;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, p::v2{style.FramePadding.x, padding});
		ImGui::PushStyleVar(
		    ImGuiStyleVar_ItemSpacing, p::v2{style.ItemSpacing.x, settings.verticalPadding});
	}

	void PopInnerNodeStyle()
	{
		ImGui::PopStyleVar(2);
	}

	void DrawReturns(p::TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>,
	                     ast::CChild, ast::CFileRef, ast::CStmtReturn, ast::CExprInputs,
	                     ast::CInvalid, ast::CExprTypeId, ast::CNamespace>
	                     scope,
	    const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CStmtReturn>(scope, children))
		{
			DrawReturnNode(scope, id);
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

	void DrawIfs(p::TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>, ast::CChild,
	                 ast::CFileRef, ast::CStmtIf, ast::CStmtOutputs, ast::CExprInputs, ast::CParent,
	                 ast::CExprTypeId>
	                 scope,
	    const TArray<ast::Id>& children)
	{
		PushNodeBackgroundColor(UI::GetNeutralColor(0));
		PushNodeTitleColor(flowColor);
		for (ast::Id id :
		    FindIdsWith<ast::CStmtIf, ast::CExprInputs, ast::CStmtOutputs>(scope, children))
		{
			BeginNode(scope, id);
			{
				Nodes::BeginNodeTitleBar();
				{
					UI::BeginGroup();
					{
						PushExecutionPinStyle();
						Nodes::BeginInput(i32(id.value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("");
						Nodes::EndInput();
						PopExecutionPinStyle();

						auto& inputs = scope.Get<const ast::CExprInputs>(id);
						if (!P_Ensure(inputs.pinIds.Size() == 1))
						{
							continue;
						}
						BeginExprInput(scope, inputs.pinIds[0], false);
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
						auto& outputs = scope.Get<const ast::CStmtOutputs>(id);
						if (!P_Ensure(outputs.pinIds.Size() == 2))
						{
							continue;
						}
						PushExecutionPinStyle();
						Nodes::BeginOutput(
						    i32(outputs.pinIds[0].value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("true");
						Nodes::EndOutput();
						Nodes::BeginOutput(
						    i32(outputs.pinIds[1].value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("false");
						Nodes::EndOutput();
						PopExecutionPinStyle();
					}
					UI::EndGroup();
				}
				Nodes::EndNodeTitleBar();
			}
			EndNode(scope);
		}
		PopNodeTitleColor();
		PopNodeBackgroundColor();
	}

	void DrawFors(p::TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>, ast::CChild,
	                  ast::CFileRef, ast::CStmtFor, ast::CStmtOutputs, ast::CExprInputs,
	                  ast::CExprOutputs, ast::CExprTypeId>
	                  scope,
	    const TArray<ast::Id>& children)
	{
		PushNodeBackgroundColor(UI::GetNeutralColor(0));
		PushNodeTitleColor(flowColor);
		for (ast::Id id :
		    FindIdsWith<ast::CStmtFor, ast::CExprInputs, ast::CStmtOutputs, ast::CExprOutputs>(
		        scope, children))
		{
			BeginNode(scope, id);
			{
				Nodes::BeginNodeTitleBar();
				{
					PushExecutionPinStyle();
					Nodes::BeginInput(i32(id.value), Nodes::PinShape_TriangleFilled);
					UI::TextUnformatted("");
					Nodes::EndInput();
					PopExecutionPinStyle();

					UI::SameLine();
					UI::BeginGroup();
					UI::TextUnformatted("for");
					UI::EndGroup();

					UI::SameLine();
					UI::BeginGroup();
					{
						auto& outputs = scope.Get<const ast::CStmtOutputs>(id);
						if (!P_Ensure(outputs.pinIds.Size() == 2))
						{
							continue;
						}
						PushExecutionPinStyle();
						Nodes::BeginOutput(
						    i32(outputs.pinIds[0].value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("each");
						Nodes::EndOutput();
						Nodes::BeginOutput(
						    i32(outputs.pinIds[1].value), Nodes::PinShape_TriangleFilled);
						UI::TextUnformatted("continue");
						Nodes::EndOutput();
						PopExecutionPinStyle();
					}
					UI::EndGroup();
				}
				Nodes::EndNodeTitleBar();

				// Inputs
				UI::BeginGroup();
				{
					auto& inputs = scope.Get<const ast::CExprInputs>(id);
					if (P_Ensure(inputs.pinIds.Size() == 2))
					{
						BeginExprInput(scope, inputs.pinIds[0], false);
						UI::TextUnformatted("first");
						EndExprInput(false);

						BeginExprInput(scope, inputs.pinIds[1], false);
						UI::TextUnformatted("end");
						EndExprInput(false);
					}
				}
				UI::EndGroup();
				UI::SameLine();

				// Outputs
				UI::BeginGroup();
				{
					auto& outputs = scope.Get<const ast::CExprOutputs>(id);
					if (P_Ensure(!outputs.pinIds.IsEmpty()))
					{
						BeginExprOutput(scope, outputs.pinIds[0], false);
						UI::TextUnformatted("index");
						EndExprOutput(false);
					}
				}
				UI::EndGroup();
			}
			EndNode(scope);
		}
		PopNodeTitleColor();
		PopNodeBackgroundColor();
	}

	void DrawUnaryOperators(
	    TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>, ast::CChild,
	        ast::CParent, ast::CFileRef, ast::CExprUnaryOperator, ast::CExprTypeId>
	        scope,
	    const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CExprUnaryOperator>(scope, children))
		{
			static constexpr Color color = UI::GetNeutralColor(0);

			PushNodeBackgroundColor(color);

			BeginNode(scope, id);
			{
				BeginExprInput(scope, id, false);
				UI::TextUnformatted("");
				EndExprInput(false);
				UI::SameLine();

				const auto& op       = scope.Get<const ast::CExprUnaryOperator>(id);
				StringView shortName = editor::GetUnaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				BeginExprOutput(scope, id, false);
				UI::TextUnformatted("");
				EndExprOutput(false);
			}
			EndNode(scope);
			PopNodeBackgroundColor();
		}
	}

	void DrawBinaryOperators(
	    TIdScopeRef<Writes<CNodePosition, ast::CChanged, ast::CFileDirty>, ast::CChild,
	        ast::CParent, ast::CFileRef, ast::CExprBinaryOperator, ast::CExprTypeId>
	        scope,
	    const TArray<ast::Id>& children)
	{
		TArray<ast::Id> pinIds;
		for (ast::Id id : FindIdsWith<ast::CExprBinaryOperator>(scope, children))
		{
			static constexpr Color color = UI::GetNeutralColor(0);
			PushNodeBackgroundColor(color);

			BeginNode(scope, id);
			{
				pinIds.Clear(p::Shrink::No);
				p::GetIdChildren(scope, id, pinIds);
				if (!P_Ensure(pinIds.Size() >= 2))
				{
					continue;
				}
				UI::BeginGroup();
				BeginExprInput(scope, pinIds[0], false);
				UI::TextUnformatted("");
				EndExprInput(false);

				BeginExprInput(scope, pinIds[1], false);
				UI::TextUnformatted("");
				EndExprInput(false);
				UI::EndGroup();
				UI::SameLine();

				const auto& op       = scope.Get<const ast::CExprBinaryOperator>(id);
				StringView shortName = editor::GetBinaryOperatorName(op.type);
				UI::Text(shortName);

				UI::SameLine();
				BeginExprOutput(scope, id, false);
				UI::TextUnformatted("");
				EndExprOutput(false);
			}
			EndNode(scope);
			PopNodeBackgroundColor();
		}
	}

	// Places the casts spawned from a link so their output pin lands on the pin they feed.
	// Both pins are known once the node has been drawn
	static void SettlePendingCasts(ast::Tree& ast)
	{
		if (pendingCasts.IsEmpty())
		{
			return;
		}

		for (i32 i = pendingCasts.Size() - 1; i >= 0; --i)
		{
			const PendingCast pending = pendingCasts[i];
			if (!ast.Has<ast::CExprCast>(pending.castId))
			{
				pendingCasts.RemoveAtSwap(i);
				continue;
			}

			v2 outputPosition;
			v2 inputPosition;
			Color color;
			if (!Nodes::GetPinScreenData(
			        i32(pending.outputPinId.value), Nodes::PinType::Output, outputPosition, color)
			    || !Nodes::GetPinScreenData(
			        i32(pending.inputPinId.value), Nodes::PinType::Input, inputPosition, color))
			{
				continue;    // The pins aren't drawn yet
			}
			pendingCasts.RemoveAtSwap(i);

			const float zoom = Nodes::GetZoom();
			if (zoom <= 0.f)
			{
				continue;
			}

			// Move the node by the distance between both pins, so they end up overlapping, and
			// keep it on the grid
			const v2 origin = Nodes::GetNodeGridSpacePos(pending.castId);
			const v2 offset =
			    (origin + (inputPosition - outputPosition) / zoom) * settings.GetInvGridSize();
			const v2 position = v2{p::Round(offset.x), p::Round(offset.y)};
			ast.GetOrAdd<CNodePosition>(pending.castId).position = position;
			SetNodePosition(pending.castId, position);
		}
	}

	void DrawCasts(ast::Tree& ast, const TArray<ast::Id>& children)
	{
		for (ast::Id id : FindIdsWith<ast::CExprCast>(ast, children))
		{
			const auto* inputs  = ast.TryGet<const ast::CExprInputs>(id);
			const auto* outputs = ast.TryGet<const ast::CExprOutputs>(id);
			if (!inputs || inputs->pinIds.IsEmpty() || !outputs || outputs->pinIds.IsEmpty())
			{
				continue;
			}

			static constexpr Color color = UI::GetNeutralColor(0);
			PushNodeBackgroundColor(color);

			BeginNode(ast, id);
			{
				// The node keeps a 3x1 footprint, so the pins sit on its corners and the
				// arrow in the middle. The font is scaled to the tile height.
				const float zoom          = Nodes::GetZoom();
				const float gridSize      = settings.GetGridSize() * zoom;
				const v2 padding          = Nodes::GetStyle().NodePadding * zoom;
				const float contentWidth  = p::Max(gridSize * 3.f - padding.x * 2.f, 1.f);
				const float contentHeight = p::Max(gridSize - padding.y * 2.f, 1.f);

				const ImVec2 start = ImGui::GetCursorPos();
				ImGui::PushFont(ImGui::GetFont(), contentHeight);

				BeginExprInput(ast, inputs->pinIds[0], false);
				UI::TextUnformatted("");
				EndExprInput(false);

				// The arrow shows the cast, colored as the type it produces
				const float arrowWidth = UI::CalcTextSize(ICON_FA_LONG_ARROW_ALT_RIGHT).x;
				ImGui::SetCursorPos(
				    ImVec2{start.x + p::Max((contentWidth - arrowWidth) * 0.5f, 0.f), start.y});
				UI::PushTextColor(GetTypeColor(ast, GetPinType(ast, outputs->pinIds[0])));
				UI::Text(ICON_FA_LONG_ARROW_ALT_RIGHT);
				UI::PopTextColor();

				// The output pin closes the node, which fixes its width
				ImGui::SetCursorPos(ImVec2{start.x + contentWidth, start.y});
				BeginExprOutput(ast, outputs->pinIds[0], false);
				UI::TextUnformatted("");
				EndExprOutput(false);

				// The node is as big as the group ends, so the cursor is left at the bottom
				// right corner of the tile area. Otherwise the item spacing would add up
				ImGui::SetCursorPos(ImVec2{start.x + contentWidth, start.y + contentHeight});

				ImGui::PopFont();
			}
			EndNode(ast);
			PopNodeBackgroundColor();
		}
	}

	void DrawStatementLinks(
	    p::TIdScopeRef<ast::CParent, ast::CStmtOutput, ast::CStmtOutputs>& scope,
	    const TArray<ast::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 2.f);
		Nodes::PushStyleColor(Nodes::ColorVar_Link, executionColor);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, UI::ToHovered(executionColor));
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, selectedColor);

		for (ast::Id outputId : FindIdsWith<ast::CStmtOutput>(scope, children))
		{
			const auto* output = scope.TryGet<const ast::CStmtOutput>(outputId);
			if (output && scope.IsValid(output->linkInputNode))
			{
				// Input pin ids equal input node ids
				// Output pin ids equal output node ids
				Nodes::Link(i32(output->linkInputNode.value), i32(outputId.value),
				    i32(output->linkInputNode.value));
			}
		}

		for (ast::Id outputId : FindIdsWith<ast::CStmtOutputs>(scope, children))
		{
			if (const auto* outputs = scope.TryGet<const ast::CStmtOutputs>(outputId))
			{
				if (P_EnsureMsg(outputs->linkInputNodes.Size() == outputs->pinIds.Size(),
				        "Inputs and pins must match. Graph might be corrupted."))
				{
					for (i32 i = 0; i < outputs->linkInputNodes.Size(); ++i)
					{
						const ast::Id outputPinId = outputs->pinIds[i];
						const ast::Id inputNodeId = outputs->linkInputNodes[i];
						if (scope.IsValid(outputPinId) && scope.IsValid(inputNodeId))
						{
							// Input pin ids equal input node ids
							// TODO: Execution pin ids atm are the same as the node id.
							// Implement proper output pin support
							Nodes::Link(i32(inputNodeId.value), i32(outputPinId.value),
							    i32(inputNodeId.value));
						}
					}
				}
			}
		}
		Nodes::PopStyleColor(3);
		Nodes::PopStyleVar();
	}

	void DrawExpressionLinks(
	    TIdScopeRef<ast::CParent, ast::CExprInputs, ast::CExprTypeId, ast::CInvalid>& scope,
	    const TArray<ast::Id>& children)
	{
		Nodes::PushStyleVar(Nodes::StyleVar_LinkThickness, 1.5f);
		Nodes::PushStyleColor(Nodes::ColorVar_LinkSelected, selectedColor);

		for (ast::Id nodeId : FindIdsWith<ast::CExprInputs>(scope, children))
		{
			const auto& inputs = scope.Get<const ast::CExprInputs>(nodeId);
			if (inputs.pinIds.Size() != inputs.linkedOutputs.Size()) [[unlikely]]
			{
				p::Warning("Corrupted expression '{}' ({} pins vs {} links). Skipping its links.",
				    nodeId, inputs.pinIds.Size(), inputs.linkedOutputs.Size());
				continue;
			}

			for (i32 i = 0; i < inputs.linkedOutputs.Size(); ++i)
			{
				ast::Id inputId        = inputs.pinIds[i];
				ast::ExprOutput output = inputs.linkedOutputs[i];
				if (!scope.IsValid(inputId) || !scope.IsValid(output.pinId))
				{
					continue;
				}

				Color color = GetTypeColor<void>();
				if (scope.Has<ast::CInvalid>(inputId) || scope.Has<ast::CInvalid>(output.pinId))
				{
					color = invalidColor;
				}
				else if (const auto* type = scope.TryGet<const ast::CExprTypeId>(output.pinId))
				{
					color = GetTypeColor(static_cast<ast::Tree&>(scope.GetContext()), type->id);
				}
				else if (const auto* type = scope.TryGet<const ast::CExprTypeId>(inputId))
				{
					color = GetTypeColor(static_cast<ast::Tree&>(scope.GetContext()), type->id);
				}

				Nodes::PushStyleColor(Nodes::ColorVar_Link, color);
				Nodes::PushStyleColor(Nodes::ColorVar_LinkHovered, UI::ToHovered(color));

				Nodes::Link(i32(inputId.value), i32(output.pinId.value), i32(inputId.value));

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
		p::FormatTo(graphId, "Graph##{}", typeId);

		bool wantsToOpenContextMenu = false;
		typeEditor.layout.BindNextWindowToNode(CTypeEditor::centralNode, ImGuiCond_Appearing);
		if (UI::Begin(graphId.c_str(), &typeEditor.showGraph, ImGuiWindowFlags_NoCollapse))
		{
			Nodes::SetEditorContext(&typeEditor.nodesEditor);
			if (UI::IsWindowAppearing())
			{
				Nodes::SetZoom(GetUserSettings<EditorSettings>().zoom.current);
			}
			Nodes::GetCurrentContext()->canCreateLinks = ast::HasFunctionBodies(ast, typeId);
			{
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
				DrawFors(ast, children);
				DrawUnaryOperators(ast, children);
				DrawBinaryOperators(ast, children);
				DrawCasts(ast, children);

				// Links
				DrawStatementLinks(ast, children);
				DrawExpressionLinks(ast, children);

				Nodes::DrawMiniMap(0.2f, Nodes::MiniMapCorner::TopRight);
				PopNodeStyle();
				Nodes::EndNodeEditor();

				// Now that the pins are drawn, casts spawned from a link can be attached to
				// the pin they feed
				SettlePendingCasts(ast);
			}

			// The focused graph stores the current zoom; the pending write is flushed even if
			// focus moves elsewhere before it settles.
			PersistZoom();

			if (UI::IsKeyReleased(ImGuiKey_Delete))
			{
				// Remove both selected and hovered links, so a link can be deleted without
				// having to box-select it first
				TArray<ast::Id> linksToRemove;
				Nodes::GetSelectedLinks(linksToRemove);
				const ast::Id hoveredLinkId = Nodes::GetHoveredLink();
				if (!IsNone(hoveredLinkId))
				{
					linksToRemove.AddUnique(hoveredLinkId);
				}
				if (!linksToRemove.IsEmpty())
				{
					ScopedChange(ast, linksToRemove);
					for (ast::Id linkId : linksToRemove)
					{
						ast::DisconnectStmtLink(ast, linkId);
						ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, linkId));
					}
				}
				ast::RemoveNodes(ast, Nodes::GetSelectedNodes());
			}

			// Move selected nodes in the pressed direction, one grid tile at a time
			const ImGuiKey directionKeys[4] = {
			    ImGuiKey_UpArrow, ImGuiKey_DownArrow, ImGuiKey_LeftArrow, ImGuiKey_RightArrow};
			const v2 directionDeltas[4] = {
			    {0.f,  -1.f},
                {0.f,  1.f },
                {-1.f, 0.f },
                {1.f,  0.f }
            };
			const bool fastMove =
			    UI::IsKeyDown(ImGuiKey_LeftShift) || UI::IsKeyDown(ImGuiKey_RightShift);
			const float step = fastMove ? fastMoveTiles : 1.f;
			for (int i = 0; i < 4; ++i)
			{
				if (UI::IsKeyPressed(directionKeys[i]))
				{
					const TArray<ast::Id>& selectedNodes = Nodes::GetSelectedNodes();
					if (!selectedNodes.IsEmpty())
					{
						ScopedChange(ast, selectedNodes);
						for (ast::Id nodeId : selectedNodes)
						{
							const v2 newPosition =
							    GetNodePosition(nodeId) + directionDeltas[i] * step;
							SetNodePosition(nodeId, newPosition);
							ast.GetOrAdd<CNodePosition>(nodeId).position = newPosition;
						}
					}
				}
			}

			Nodes::Id outputPin;
			Nodes::Id inputPin;
			if (Nodes::IsLinkCreated(outputPin, inputPin))
			{
				const ast::Id outputPinId = ast::Id::MakeRaw(outputPin);
				const ast::Id inputPinId  = ast::Id::MakeRaw(inputPin);
				ast::Id pinIds[2]{outputPinId, inputPinId};
				ScopedChange(ast, pinIds);
				ast::TryConnectStmt(ast, outputPinId, inputPinId);

				const ast::ExprOutput output = ast::GetExprOutputFromPin(ast, outputPinId);
				const ast::ExprInput input   = ast::GetExprInputFromPin(ast, inputPinId);
				if (!ast::TryConnectExpr(ast, output, input))
				{
					// The value needs an explicit cast, which goes in a node of its own placed
					// over the pin it feeds
					const ast::Id castId = InsertCastNode(ast, typeId, output, input);
					if (!p::IsNone(castId))
					{
						const auto* castOutputs = ast.TryGet<const ast::CExprOutputs>(castId);
						if (castOutputs && !castOutputs->pinIds.IsEmpty())
						{
							// Rough position until the node is drawn and its pins are known
							const v2 pinPosition = settings.GetGridPosition(UI::GetMousePos());
							ast.Add<CNodePosition>(castId, pinPosition - v2(3.f, 0.5f));
							pendingCasts.Add({castId, castOutputs->pinIds[0], inputPinId});
						}
					}
				}
			}
			Nodes::Id linkId;
			if (Nodes::IsLinkDestroyed(linkId))
			{
				ScopedChange(ast, ast::Id::MakeRaw(linkId));
				// linkId is always the outputId
				ast::DisconnectStmtLink(ast, ast::Id::MakeRaw(linkId));
				ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, ast::Id::MakeRaw(linkId)));
			}

			// A link dropped over empty canvas opens the context menu instead of silently
			// doing nothing, so the user can pick the node to connect it to.
			Nodes::Id droppedOutput;
			Nodes::Id droppedInput;
			if (Nodes::IsLinkDropped(&droppedOutput, &droppedInput))
			{
				const v2 releasePos = UI::GetMousePos();
				if (droppedOutput != 0)
				{
					typeEditor.menuContext = {
					    ast::Id::MakeRaw(droppedOutput), Nodes::PinType::Output, releasePos};
				}
				else if (droppedInput != 0)
				{
					typeEditor.menuContext = {
					    ast::Id::MakeRaw(droppedInput), Nodes::PinType::Input, releasePos};
				}
				wantsToOpenContextMenu = typeEditor.menuContext.HasSource();
			}

			// Keep the dropped link visible under the context menu until it closes
			{
				v2 startPos;
				Color startColor;
				if (typeEditor.menuContext.HasSource()
				    && Nodes::GetPinScreenData(i32(typeEditor.menuContext.sourcePin.value),
				        typeEditor.menuContext.type, startPos, startColor))
				{
					// Keep the link anchored where it was released instead of following
					// the mouse, so it stays put while the menu is open
					const v2 endPos                 = typeEditor.menuContext.location;
					const Nodes::CubicBezier bezier = Nodes::MakeCubicBezier(startPos, endPos,
					    typeEditor.menuContext.type, Nodes::GetStyle().linkLineSegmentsPerLength);
					ImGui::GetWindowDrawList()->AddBezierCubic(bezier.p0, bezier.p1, bezier.p2,
					    bezier.p3, startColor.ToPackedABGR(), Nodes::GetStyle().LinkThickness,
					    bezier.numSegments);
				}
			}

			// Show the hovered pin's type in a tooltip
			Nodes::Id hoveredPinId;
			Nodes::PinType hoveredPinType;
			if (Nodes::GetHoveredPin(hoveredPinId, hoveredPinType))
			{
				const ast::Id pinId = ast::Id::MakeRaw(hoveredPinId);
				StringView typeName;
				if (const auto* typeId = ast.TryGet<const ast::CExprTypeId>(pinId))
				{
					if (const auto* typeNs = ast.TryGet<const ast::CNamespace>(typeId->id))
					{
						typeName = typeNs->name.AsString();
					}
				}
				else
				{
					typeName = "Statement";
				}
				if (typeName.empty())
				{
					typeName = "?";
				}
				UI::SetTooltip(typeName);
			}

			ast::Id hoveredNodeId = Nodes::GetHoveredNode();
			if (!IsNone(hoveredNodeId) && Nodes::IsNodeSelected(hoveredNodeId)
			    && UI::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (ast.Has<ast::CStmtReturn>(hoveredNodeId))
				{
					// A return node has no details of its own. Instead show the function it
					// returns into, and only when it's connected to one.
					const ast::Id functionId =
					    ast::FunctionsSystem::FindOwningFunction(ast, hoveredNodeId);
					if (!IsNone(functionId))
					{
						typeEditor.selectedPropertyId = functionId;
					}
				}
				else
				{
					typeEditor.selectedPropertyId = hoveredNodeId;
				}
			}

			if (wantsToOpenContextMenu)
			{
				if (typeEditor.menuContext.HasSource())
				{
					// Always show the (filtered) graph menu when a link was dropped, even
					// if the cursor ended over a node
					typeEditor.menuContext.hoveredNodeId = ast::NoId;
					typeEditor.menuContext.hoveredLinkId = ast::NoId;
				}
				else
				{
					typeEditor.menuContext.hoveredNodeId = Nodes::GetHoveredNode();
					typeEditor.menuContext.hoveredLinkId = Nodes::GetHoveredLink();
				}
				ImGui::OpenPopup("ContextMenu", ImGuiPopupFlags_AnyPopup);
			}
			DrawContextMenu(ast, typeId, typeEditor.menuContext);

			// Forget the pin that was dropped once the menu goes away
			if (!ImGui::IsPopupOpen("ContextMenu"))
			{
				typeEditor.menuContext = {};
			}
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
