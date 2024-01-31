// Copyright 2015-2023 Piperift - All rights reserved

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
#include <AST/Utils/TypeUtils.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/EnumFlags.h>
#include <PipeECS.h>
#include <UI/UI.h>


namespace rift::Editor
{
	void EditFunctionPin(ast::Tree& ast, ast::Id ownerId, ast::Id id)
	{
		auto* ns   = ast.TryGet<ast::CNamespace>(id);
		auto* type = ast.TryGet<ast::CExprType>(id);
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

		ast::Id typeId = ast::FindIdFromNamespace(ast, type->type);

		UI::TableNextRow();
		const Color color = GetTypeColor(ast, typeId);
		UI::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color.DWColor());

		UI::TableNextColumn();    // Name
		labelId.clear();
		Strings::FormatTo(labelId, "##Name_{}", id);
		String name{ns->name.AsString()};
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::MutableText(labelId, name, ImGuiInputTextFlags_AutoSelectAll))
		{
			ScopedChange(ast, id);
			ns->name = Tag{name};
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
			ast.GetOrAdd<ast::CExprTypeId>(id).id = typeId;
			type->type                            = ast::GetNamespace(ast, typeId);
		}
		UI::PopStyleVar();
		if (UI::IsItemHovered())
		{
			hovered = true;
		}

		if (hovered)
		{
			if (UI::IsKeyReleased(ImGuiKey_Delete))
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
			ast::RemoveExprInputPin(ast, ast::GetExprInputFromPin(ast, id));
			ast::RemoveExprOutputPin(ast, ast::GetExprOutputFromPin(ast, id));
		}
	}

	void DrawFunction(ast::Tree& ast, ast::Id typeId, ast::Id id)
	{
		auto* ns = ast.TryGet<ast::CNamespace>(id);
		if (!ns)
		{
			return;
		}

		String functionName{ns->name.AsString()};
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::InputText("##name", functionName, ImGuiInputTextFlags_AutoSelectAll))
		{
			Id sameNameFuncId = ast::FindChildByName(ast, typeId, Tag{functionName});
			if (!IsNone(sameNameFuncId) && id != sameNameFuncId)
			{
				UI::PushTextColor(LinearColor::Red());
				UI::SetTooltip("This name is in use by another function in this type");
				UI::PopTextColor();
			}
			else
			{
				ScopedChange(ast, id);
				ns->name = Tag{functionName};
			}
		}
		UI::Spacing();

		UI::Text("Inputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprOutputs = ast.TryGet<const ast::CExprOutputs>(id))
			{
				for (ast::Id pinId : exprOutputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		UI::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::Selectable(ICON_FA_PLUS "##AddInput"))
		{
			ScopedChange(ast, id);
			ast::AddFunctionInput(ast, id);
		}
		UI::HelpTooltip("Adds a new input parameter to a function");
		UI::PopStyleVar();
		UI::PopStyleCompact();
		UI::Spacing();

		UI::Text("Outputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprInputs = ast.TryGet<const ast::CExprInputs>(id))
			{
				for (ast::Id pinId : exprInputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		UI::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::Selectable(ICON_FA_PLUS "##AddOutput"))
		{
			ScopedChange(ast, id);
			ast::AddFunctionOutput(ast, id);
		}
		UI::HelpTooltip("Adds a new output parameter to a function");
		UI::PopStyleVar();
		UI::PopStyleCompact();
		UI::Spacing();
	}

	void DrawDetailsPanel(ast::Tree& ast, ast::Id typeId)
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

			if (ast.Has<ast::CDeclFunction>(editor.selectedPropertyId))
			{
				DrawFunction(ast, typeId, editor.selectedPropertyId);
			}
		}
		UI::End();
	}
}    // namespace rift::Editor
