// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/Widgets.h"

#include <UI/UI.h>


namespace rift::editor
{
	void ListTypesFromFilter(p::TIdScopeRef<ast::CNamespace> scope, p::TArray<ast::Id> typeIds,
	    ast::Id& selectedId, ImGuiTextFilter& searchFilter)
	{
		for (ast::Id id : typeIds)
		{
			const auto& type   = scope.Get<const ast::CNamespace>(id);
			p::StringView name = type.name.AsString();

			if (!searchFilter.PassFilter(name.data(), name.data() + name.size()))
			{
				continue;
			}

			UI::PushID(id.value);
			if (UI::Selectable(name.data(), id == selectedId))
			{
				selectedId = id;
			}
			UI::PopID();
		}
	}

	bool TypeCombo(p::TIdScopeRef<ast::CNamespace, ast::CDeclType, ast::CDeclNative,
	                   ast::CDeclStruct, ast::CDeclClass>
	                   scope,
	    p::StringView label, ast::Id& selectedId)
	{
		p::Tag ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = scope.Get<const ast::CNamespace>(selectedId).name;
		}

		ast::Id lastId = selectedId;
		if (UI::BeginCombo(label.data(), ownerName.AsString().data()))
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
			if (filter.IsActive())
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
		return selectedId != lastId;
	}

	bool InputLiteralValue(ast::Tree& ast, p::StringView label, ast::Id literalId)
	{
		return false;
	}
}    // namespace rift::editor
