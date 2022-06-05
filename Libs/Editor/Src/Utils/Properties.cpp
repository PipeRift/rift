// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/Properties.h"

#include "AST/Id.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/Nodes.h"
#include "Utils/Widgets.h"

#include <AST/Utils/Expressions.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TypeUtils.h>
#include <Core/EnumFlags.h>
#include <ECS/Filtering.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <UI/UI.h>


namespace rift
{
	using namespace EnumOperators;


	void DrawVariable(TVariableAccessRef access, CTypeEditor& editor, AST::Id variableId)
	{
		CIdentifier* identifier = access.TryGet<CIdentifier>(variableId);
		auto* variableDecl      = access.TryGet<CDeclVariable>(variableId);
		if (!identifier || !variableDecl)
		{
			return;
		}

		ImGui::PushID(identifier);

		const Color color =
		    Style::GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), variableDecl->typeId);
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			Style::PushHeaderColor(LinearColor::Transparent());

			ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = editor.selectedPropertyId == variableId;
			ImGui::Selectable("##selectArea", &selected,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			    ImVec2(0, frameHeight));

			if (selected)
			{
				editor.selectedPropertyId = variableId;

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					editor.pendingDeletePropertyId = variableId;
				}
			}
			else if (editor.selectedPropertyId == variableId)    // If not selected but WAS selected
			{
				editor.selectedPropertyId = AST::NoId;
			}

			Color bgColor = color;
			if (selected)
			{
				bgColor = color.Tint(0.1f);
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 2.f);

			Style::PopHeaderColor();
		}

		if (UI::BeginPopupContextItem())
		{
			if (UI::MenuItem("Delete"))
			{
				editor.pendingDeletePropertyId = variableId;
			}
			UI::EndPopup();
		}

		UI::SameLine();

		UI::SetNextItemWidth(-FLT_MIN);
		static String nameId;
		nameId.clear();
		Strings::FormatTo(nameId, " ##name_{}", variableId);
		String name = identifier->name.ToString();
		if (UI::MutableText(nameId, name,
		        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			identifier->name = Name{name};
		}

		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
			UI::SetNextItemWidth(-FLT_MIN);
			Editor::TypeCombo(access, "##type", variableDecl->typeId);
			UI::PopStyleVar();
		}

		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{4.f, 4.f});
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			static float value = 2.f;
			UI::SetNextItemWidth(-FLT_MIN);
			UI::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.2f");
			UI::PopStyleVar(2);
		}

		UI::PopID();
	}

	void DrawFunction(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId, AST::Id id)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(id);
		if (!identifier)
		{
			return;
		}

		String functionName = identifier->name.ToString();

		Style::PushHeaderColor(Style::callColor);
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
		static String headerId;
		headerId.clear();
		Strings::FormatTo(headerId, "{}###{}", functionName, id);
		UI::CollapsingHeader(headerId.c_str(), ImGuiTreeNodeFlags_Leaf);
		if (UI::IsItemHovered() && UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			Nodes::MoveToNode(id, v2{150.f, 150.f});
		}
		UI::PopStyleVar();
		Style::PopHeaderColor();

		if (ImGui::BeginPopupContextItem())
		{
			Graph::DrawNodesContextMenu(ast, typeId, id);
			if (UI::MenuItem("Show in Graph"))
			{
				Nodes::MoveToNode(id, v2{150.f, 150.f});
			}
			ImGui::EndPopup();
		}
	}

	void DrawVariables(TVariableAccessRef access, TransactionAccess transAccess,
	    CTypeEditor& editor, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen))
		{
			UI::Indent(10.f);
			TArray<AST::Id> variableIds;
			AST::Hierarchy::GetChildren(access, typeId, variableIds);
			ECS::ExcludeIfNot<CDeclVariable>(access, variableIds);

			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
			bool showTable = UI::BeginTable("##variableTable", 3, ImGuiTableFlags_SizingFixedFit);
			UI::PopStyleVar();
			if (showTable)
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.45f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.25f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.30f);
				for (AST::Id child : variableIds)
				{
					if (access.Has<CDeclVariable>(child))
					{
						UI::TableNextRow();
						DrawVariable(access, editor, child);
					}
				}
				UI::EndTable();
			}

			Style::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(transAccess, typeId);
				Types::AddVariable(
				    {static_cast<AST::Tree&>(access.GetContext()), typeId}, "NewVariable");
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawFunctions(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId)
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
		                               | ImGuiTreeNodeFlags_AllowItemOverlap
		                               | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
		if (UI::CollapsingHeader("Functions", flags))
		{
			UI::Indent(10.f);

			TArray<AST::Id> functionIds;
			AST::Hierarchy::GetChildren(ast, typeId, functionIds);
			ECS::ExcludeIfNot<CDeclFunction>(ast, functionIds);
			for (AST::Id functionId : functionIds)
			{
				DrawFunction(ast, editor, typeId, functionId);
			}

			Style::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(ast, typeId);
				Types::AddFunction({ast, typeId}, "NewFunction");
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawProperties(AST::Tree& ast, AST::Id typeId)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		const String windowName = Strings::Format("Properties##{}", typeId);
		if (UI::Begin(windowName.c_str()))
		{
			if (Types::CanContainVariables(ast, typeId))
			{
				DrawVariables(ast, ast, editor, typeId);
			}

			if (Types::CanContainFunctions(ast, typeId))
			{
				DrawFunctions(ast, editor, typeId);
			}
		}
		UI::End();

		if (!IsNone(editor.pendingDeletePropertyId))
		{
			ScopedChange(ast, editor.pendingDeletePropertyId);
			bool removedPin = AST::Expressions::RemoveInputPin(
			    ast, AST::Expressions::InputFromPinId(ast, editor.pendingDeletePropertyId));
			removedPin |= AST::Expressions::RemoveOutputPin(
			    ast, AST::Expressions::OutputFromPinId(ast, editor.pendingDeletePropertyId));

			// If pin has not been marked for removal, destroy the entity
			if (!removedPin)
			{
				AST::Hierarchy::RemoveDeep(ast, editor.pendingDeletePropertyId);
				editor.pendingDeletePropertyId = AST::NoId;
			}
		}
	}
}    // namespace rift
