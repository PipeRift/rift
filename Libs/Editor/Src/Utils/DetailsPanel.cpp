// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/ElementsPanel.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/Nodes.h"
#include "Utils/Widgets.h"

#include <AST/Utils/Expressions.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/EnumFlags.h>
#include <Pipe/ECS/Filtering.h>
#include <UI/UI.h>


namespace rift
{
	void EditFunctionPin(AST::Tree& ast, AST::Id ownerId, AST::Id id)
	{
		auto* ns   = ast.TryGet<CNamespace>(id);
		auto* type = ast.TryGet<CExprType>(id);
		if (!ns || !type)
		{
			return;
		}

		bool removePin = false;
		bool hovered   = false;

		static String labelId;
		static String popupName;
		popupName.clear();
		Strings::FormatTo(popupName, "##PinContextMenu_{}", id);

		AST::Id typeId = AST::FindIdFromNamespace(ast, type->type);

		UI::TableNextRow();
		const Color color = Style::GetTypeColor(ast, typeId);
		UI::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color.DWColor());

		UI::TableNextColumn();    // Name
		labelId.clear();
		Strings::FormatTo(labelId, "##Name_{}", id);
		String name = ns->name.ToString();
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::MutableText(labelId, name, ImGuiInputTextFlags_AutoSelectAll))
		{
			ScopedChange(ast, id);
			ns->name = Name{name};
		}
		if (UI::IsItemHovered())
		{
			hovered = true;
		}

		UI::TableNextColumn();    // Type
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
		labelId.clear();
		Strings::FormatTo(labelId, "##Type_{}", id);
		UI::SetNextItemWidth(-FLT_MIN);
		if (Editor::TypeCombo(ast, labelId, typeId))
		{
			ScopedChange(ast, id);
			ast.GetOrAdd<CExprTypeId>(id).id = typeId;
			type->type                       = AST::GetNamespace(ast, typeId);
		}
		UI::PopStyleVar();
		if (UI::IsItemHovered())
		{
			hovered = true;
		}

		if (hovered)
		{
			if (UI::IsKeyReleased(GLFW_KEY_DELETE))
			{
				removePin = true;
			}
			else if (UI::IsMouseReleased(ImGuiMouseButton_Right))
			{
				UI::OpenPopup(popupName.c_str());
			}
		}
		if (UI::BeginPopup(popupName.c_str()))
		{
			if (UI::MenuItem("Delete"))
			{
				removePin = true;
			}
			UI::EndPopup();
		}
		if (removePin)
		{
			AST::Expressions::RemoveInputPin(ast, AST::Expressions::InputFromPinId(ast, id));
			AST::Expressions::RemoveOutputPin(ast, AST::Expressions::OutputFromPinId(ast, id));
		}
	}

	void DrawFunction(AST::Tree& ast, AST::Id typeId, AST::Id id)
	{
		auto* ns = ast.TryGet<CNamespace>(id);
		if (!ns)
		{
			return;
		}

		String functionName = ns->name.ToString();
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::InputText("##name", functionName, ImGuiInputTextFlags_AutoSelectAll))
		{
			ecs::Id sameNameFuncId = Types::FindFunctionByName(ast, typeId, Name{functionName});
			if (!IsNone(sameNameFuncId) && id != sameNameFuncId)
			{
				Style::PushTextColor(LinearColor::Red());
				UI::SetTooltip("This name is in use by another function in this type");
				Style::PopTextColor();
			}
			else
			{
				ScopedChange(ast, id);
				ns->name = Name{functionName};
			}
		}
		UI::Spacing();

		UI::Text("Inputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprOutputs = ast.TryGet<const CExprOutputs>(id))
			{
				for (AST::Id pinId : exprOutputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		Style::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});
		UI::SetNextItemWidth(UI::GetContentRegionAvailWidth());
		if (UI::Selectable(ICON_FA_PLUS "##AddInput"))
		{
			ScopedChange(ast, id);
			Types::AddFunctionInput(ast, id);
		}
		UI::HelpTooltip("Adds a new input parameter to a function");
		UI::PopStyleVar();
		Style::PopStyleCompact();
		UI::Spacing();

		UI::Text("Outputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit, {200.f, 0.f}))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprInputs = ast.TryGet<const CExprInputs>(id))
			{
				for (AST::Id pinId : exprInputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		Style::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		UI::SetNextItemWidth(UI::GetContentRegionAvailWidth());
		if (UI::Selectable(ICON_FA_PLUS "##AddOutput"))
		{
			ScopedChange(ast, id);
			Types::AddFunctionOutput(ast, id);
		}
		UI::HelpTooltip("Adds a new output parameter to a function");
		UI::PopStyleVar();
		Style::PopStyleCompact();
		UI::Spacing();
	}

	void DrawDetailsPanel(AST::Tree& ast, AST::Id typeId)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		if (!editor.showDetails)
		{
			return;
		}

		const String windowName = Strings::Format("Details##{}", typeId);
		if (UI::Begin(windowName.c_str(), &editor.showDetails))
		{
			if (IsNone(editor.selectedPropertyId))
			{
				UI::End();
				return;
			}

			if (ast.Has<CDeclFunction>(editor.selectedPropertyId))
			{
				DrawFunction(ast, typeId, editor.selectedPropertyId);
			}
		}
		UI::End();
	}
}    // namespace rift
