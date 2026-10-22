// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/Widgets.h"

#include "Utils/EditorStyle.h"
#include "Utils/TypeUtils.h"

#include <UI/UI.h>


namespace rift::editor
{
	void ListTypesFromFilter(p::TIdScopeRef<ast::CNamespace> scope, p::TArray<ast::Id> typeIds,
	    ast::Id& selectedId, ImGuiTextFilter& searchFilter)
	{
		ast::Tree& ast        = static_cast<ast::Tree&>(scope.GetContext());
		const float rowHeight = UI::GetTextLineHeight() + 6.f;
		for (ast::Id id : typeIds)
		{
			const auto& type = scope.Get<const ast::CNamespace>(id);
			StringView name  = type.name.AsString();

			if (!searchFilter.PassFilter(name.data(), name.data() + name.size()))
			{
				continue;
			}

			UI::PushID(id);
			ImVec2 rowMin;
			ImVec2 rowMax;
			const bool pressed = UI::SelectableRow("###Type", rowMin, rowMax,
			    p::LinearColor::Transparent(), rowHeight, id == selectedId);
			// The type is drawn as a chip over the row
			const StringView icon = GetTypeIcon(ast, id);
			float x               = rowMin.x + UI::chipPadding;
			UI::DrawChip(*UI::GetWindowDrawList(), x, (rowMin.y + rowMax.y) * 0.5f, name,
			    GetTypeColor(ast, id), icon.empty() ? nullptr : icon.data());
			UI::PopID();
			if (pressed)
			{
				selectedId = id;
			}
		}
	}

	bool TypeCombo(p::TIdScopeRef<ast::CNamespace, ast::CDeclType, ast::CDeclNative,
	                   ast::CDeclStruct, ast::CDeclClass>
	                   scope,
	    p::StringView label, ast::Id& selectedId, bool selectedAsChip, bool showCategories)
	{
		p::Tag ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = scope.Get<const ast::CNamespace>(selectedId).name;
		}
		const StringView ownerDisplayName = ownerName.AsString();

		// The selected type can be drawn as a chip inside the combo, like the dropdown items
		const bool previewAsChip = selectedAsChip && !IsNone(selectedId);
		ImGuiComboFlags flags    = previewAsChip ? ImGuiComboFlags_CustomPreview : 0;

		ast::Id lastId = selectedId;
		const bool open =
		    UI::BeginCombo(label.data(), previewAsChip ? nullptr : ownerDisplayName.data(), flags);
		if (open)
		{
			static ImGuiTextFilter filter;
			if (UI::IsWindowAppearing())
			{
				UI::SetKeyboardFocusHere();
			}
			UI::SetNextItemWidth(-FLT_MIN);
			filter.Draw("##Filter");

			auto nativeIds =
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclNative, ast::CNamespace>(scope);
			auto structIds =
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclStruct, ast::CNamespace>(scope);
			auto classIds =
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclClass, ast::CNamespace>(scope);
			if (!showCategories)
			{
				// A flat list of every type, sorted by name
				p::TArray<ast::Id> typeIds;
				typeIds.Append(nativeIds);
				typeIds.Append(structIds);
				typeIds.Append(classIds);
				typeIds.Sort([&scope](ast::Id a, ast::Id b)
				{
					return scope.Get<const ast::CNamespace>(a).name.AsString()
					     < scope.Get<const ast::CNamespace>(b).name.AsString();
				});
				ListTypesFromFilter(scope, typeIds, selectedId, filter);
			}
			else if (filter.IsActive())
			{
				if (UI::TreeNodeEx("Native##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(scope, nativeIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Structs##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(scope, structIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Classes##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(scope, classIds, selectedId, filter);
					UI::TreePop();
				}
			}
			else
			{
				if (UI::TreeNodeEx("Native", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(scope, nativeIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNode("Structs"))
				{
					ListTypesFromFilter(scope, structIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNode("Classes"))
				{
					ListTypesFromFilter(scope, classIds, selectedId, filter);
					UI::TreePop();
				}
			}
			UI::EndCombo();
		}

		// The custom preview is drawn right after the combo block, inside the combo's frame
		if (previewAsChip && UI::BeginComboPreview())
		{
			ast::Tree& ast        = static_cast<ast::Tree&>(scope.GetContext());
			const StringView icon = GetTypeIcon(ast, selectedId);
			const ImVec2 pos      = UI::GetCursorScreenPos();
			float x               = pos.x;
			UI::DrawChip(*UI::GetWindowDrawList(), x, pos.y + UI::GetTextLineHeight() * 0.5f,
			    ownerDisplayName, GetTypeColor(ast, selectedId),
			    icon.empty() ? nullptr : icon.data());
			UI::EndComboPreview();
		}
		return selectedId != lastId;
	}

	bool InputLiteralValue(ast::Tree& ast, p::StringView label, ast::Id literalId)
	{
		return false;
	}
}    // namespace rift::editor
