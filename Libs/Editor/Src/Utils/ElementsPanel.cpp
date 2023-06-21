// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/ElementsPanel.h"

#include "AST/Id.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/Nodes.h"
#include "Utils/Widgets.h"

#include <AST/Utils/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/EnumFlags.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/PipeECS.h>
#include <UI/UI.h>


namespace rift::Editor
{
	// using namespace EnumOperators;

	void DrawVariable(TVariableAccessRef access, CTypeEditor& editor, AST::Id variableId)
	{
		auto* ns           = access.TryGet<AST::CNamespace>(variableId);
		auto* variableDecl = access.TryGet<AST::CDeclVariable>(variableId);
		if (!ns || !variableDecl)
		{
			return;
		}
		String name{ns->name.AsString()};
		if (!editor.elementsFilter.PassFilter(name.data(), name.data() + name.size()))
		{
			return;
		}

		ImGui::PushID(ns);

		const Color color =
		    GetTypeColor(static_cast<AST::Tree&>(access.GetContext()), variableDecl->typeId);
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			UI::PushHeaderColor(LinearColor::Transparent());

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
				bgColor = UI::Hovered(color);
			}
			UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 2.f);

			UI::PopHeaderColor();
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
		if (UI::MutableText(nameId, name,
		        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ns->name = Tag{name};
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
		auto* ns = ast.TryGet<AST::CNamespace>(id);
		if (!ns)
		{
			return;
		}
		StringView name = ns->name.AsString();
		if (!editor.elementsFilter.PassFilter(name.data(), name.data() + name.size()))
		{
			return;
		}

		UI::PushHeaderColor(callColor);
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
		static String headerId;
		headerId.clear();
		Strings::FormatTo(headerId, "{}###{}", name, id);

		const bool selected = editor.selectedPropertyId == id;
		UI::CollapsingHeader(headerId.c_str(),
		    (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf);
		if (UI::IsItemHovered() && UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			Nodes::MoveToNode(id, v2{150.f, 150.f});
		}
		else if (UI::IsItemClicked())
		{
			editor.selectedPropertyId = id;
		}
		UI::PopStyleVar();
		UI::PopHeaderColor();

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

	void DrawVariables(TVariableAccessRef access, AST::TransactionAccess transAccess,
	    CTypeEditor& editor, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen))
		{
			UI::Indent(10.f);
			TArray<AST::Id> variableIds;
			p::ecs::GetChildren(access, typeId, variableIds);
			ExcludeIdsWithout<AST::CDeclVariable>(access, variableIds);

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
					if (access.Has<AST::CDeclVariable>(child))
					{
						UI::TableNextRow();
						DrawVariable(access, editor, child);
					}
				}
				UI::EndTable();
			}

			UI::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(transAccess, typeId);
				AST::AddVariable(
				    {static_cast<AST::Tree&>(access.GetContext()), typeId}, "NewVariable");
			}
			UI::PopStyleCompact();
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
			p::ecs::GetChildren(ast, typeId, functionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(ast, functionIds);
			for (AST::Id functionId : functionIds)
			{
				DrawFunction(ast, editor, typeId, functionId);
			}

			UI::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(ast, typeId);
				AST::AddFunction({ast, typeId}, "NewFunction");
			}
			UI::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawElementsPanel(AST::Tree& ast, AST::Id typeId)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		if (!editor.showElements)
		{
			return;
		}

		const String windowName = Strings::Format("Elements##{}", typeId);
		if (UI::Begin(windowName.c_str(), &editor.showElements))
		{
			UI::SetNextItemWidth(UI::GetContentRegionAvailWidth());
			editor.elementsFilter.Draw("##filter");

			if (AST::HasVariables(ast, typeId))
			{
				DrawVariables(ast, ast, editor, typeId);
			}

			if (AST::HasFunctions(ast, typeId))
			{
				DrawFunctions(ast, editor, typeId);
			}
		}
		UI::End();

		if (!IsNone(editor.pendingDeletePropertyId))
		{
			ScopedChange(ast, editor.pendingDeletePropertyId);
			bool removedPin = AST::RemoveExprInputPin(
			    ast, AST::GetExprInputFromPin(ast, editor.pendingDeletePropertyId));
			removedPin |= AST::RemoveExprOutputPin(
			    ast, AST::GetExprOutputFromPin(ast, editor.pendingDeletePropertyId));

			// If pin has not been marked for removal, destroy the entity
			if (!removedPin)
			{
				p::ecs::Remove(ast, editor.pendingDeletePropertyId, true);
				editor.pendingDeletePropertyId = AST::NoId;
			}
		}
	}
}    // namespace rift::Editor
