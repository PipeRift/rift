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
			const StringView icon = GetTypeIcon(ast, id);
			UI::SetNextItemAllowOverlap();
			ImVec2 rowMin;
			ImVec2 rowMax;
			const bool pressed = UI::SelectableRow("###Type", rowMin, rowMax,
			    p::LinearColor::Transparent(), rowHeight, id == selectedId);
			// The chip is a non-interactive item laid over the row, so the dropdown grows
			// to fit it
			UI::SetCursorScreenPos(ImVec2{rowMin.x + UI::chipPadding, rowMin.y});
			UI::Chip(name, GetTypeColor(ast, id), icon.empty() ? nullptr : icon.data());
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
	    p::StringView label, ast::Id& selectedId, TypeComboFlags flags)
	{
		p::Tag ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = scope.Get<const ast::CNamespace>(selectedId).name;
		}
		const StringView ownerDisplayName = ownerName.AsString();

		const bool selectedAsChip = p::HasFlag(flags, TypeComboFlags::SelectedAsChip);
		const bool showCategories = p::HasFlag(flags, TypeComboFlags::ShowCategories);
		const bool previewAsChip  = selectedAsChip && !IsNone(selectedId);

		ast::Tree& ast                = static_cast<ast::Tree&>(scope.GetContext());
		const StringView selectedIcon = previewAsChip ? GetTypeIcon(ast, selectedId) : StringView{};

		ast::Id lastId = selectedId;
		if (UI::BeginCombo(label.data(), previewAsChip ? nullptr : ownerDisplayName.data(),
		        previewAsChip ? ImGuiComboFlags_CustomPreview : 0))
		{
			static ImGuiTextFilter filter;
			if (UI::IsWindowAppearing())
			{
				UI::SetKeyboardFocusHere();
			}
			UI::SetNextItemWidth(-FLT_MIN);
			filter.Draw("##Filter");

			TArray<ast::Id> typeIds = p::FindAllIdsWith<ast::CDeclType, ast::CNamespace>(scope);
			if (!showCategories)
			{
				ListTypesFromFilter(scope, typeIds, selectedId, filter);
			}
			else
			{
				// Ordered by expected quantities
				TArray<ast::Id> structIds = ExtractIdsWith<ast::CDeclStruct>(scope, typeIds);
				TArray<ast::Id> classIds  = ExtractIdsWith<ast::CDeclClass>(scope, typeIds);
				ExcludeIdsWithout<ast::CDeclNative>(scope, typeIds);

				// Categories stay open while searching
				const ImGuiTreeNodeFlags categoryFlags =
				    filter.IsActive() ? ImGuiTreeNodeFlags_DefaultOpen : 0;
				if (UI::TreeNodeEx("Native", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(scope, typeIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Structs", categoryFlags))
				{
					ListTypesFromFilter(scope, structIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Classes", categoryFlags))
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
			const ImRect previewRect = UI::GetCurrentContext()->ComboPreviewData.PreviewRect;
			const float maxWidth     = previewRect.GetWidth() - UI::GetStyle().FramePadding.x * 2.f;
			UI::Chip(ownerDisplayName, GetTypeColor(ast, selectedId),
			    selectedIcon.empty() ? nullptr : selectedIcon.data(), maxWidth);
			UI::EndComboPreview();
		}
		return selectedId != lastId;
	}

	bool InputLiteralValue(ast::Tree& ast, p::StringView label, ast::Id literalId)
	{
		return false;
	}
}    // namespace rift::editor
