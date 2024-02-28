// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/Widgets.h"

#include <UI/UI.h>


namespace rift::editor
{
	void ListTypesFromFilter(p::TAccessRef<ast::CNamespace> access, p::TArray<ast::Id> typeIds,
	    ast::Id& selectedId, ImGuiTextFilter& searchFilter)
	{
		for (ast::Id id : typeIds)
		{
			const auto& type   = access.Get<const ast::CNamespace>(id);
			p::StringView name = type.name.AsString();

			if (!searchFilter.PassFilter(name.data(), name.data() + name.size()))
			{
				continue;
			}

			UI::PushID(p::u32(id));
			if (UI::Selectable(name.data(), id == selectedId))
			{
				selectedId = id;
			}
			UI::PopID();
		}
	}

	bool TypeCombo(p::TAccessRef<ast::CNamespace, ast::CDeclType, ast::CDeclNative,
	                   ast::CDeclStruct, ast::CDeclClass>
	                   access,
	    p::StringView label, ast::Id& selectedId)
	{
		p::Tag ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = access.Get<const ast::CNamespace>(selectedId).name;
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
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclNative, ast::CNamespace>(access);
			auto structIds =
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclStruct, ast::CNamespace>(access);
			auto classIds =
			    p::FindAllIdsWith<ast::CDeclType, ast::CDeclClass, ast::CNamespace>(access);
			if (filter.IsActive())
			{
				if (UI::TreeNodeEx("Native##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(access, nativeIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Structs##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(access, structIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNodeEx("Classes##Filtered", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(access, classIds, selectedId, filter);
					UI::TreePop();
				}
			}
			else
			{
				if (UI::TreeNodeEx("Native", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ListTypesFromFilter(access, nativeIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNode("Structs"))
				{
					ListTypesFromFilter(access, structIds, selectedId, filter);
					UI::TreePop();
				}
				if (UI::TreeNode("Classes"))
				{
					ListTypesFromFilter(access, classIds, selectedId, filter);
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
