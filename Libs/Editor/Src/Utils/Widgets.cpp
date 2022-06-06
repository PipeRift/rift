// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/Widgets.h"

#include <UI/UI.h>


namespace rift::Editor
{
	void ListTypesFromFilter(TAccessRef<CType> access, TArray<AST::Id> typeIds, AST::Id& selectedId,
	    ImGuiTextFilter& searchFilter)
	{
		for (AST::Id id : typeIds)
		{
			const auto& type   = access.Get<const CType>(id);
			const String& name = type.name.ToString();

			if (!searchFilter.PassFilter(name.c_str(), name.c_str() + name.size()))
			{
				continue;
			}

			UI::PushID(u32(id));
			if (UI::Selectable(name.c_str(), id == selectedId))
			{
				selectedId = id;
			}
			UI::PopID();
		}
	}

	bool TypeCombo(TAccessRef<CType, CDeclNative, CDeclStruct, CDeclClass> access, StringView label,
	    AST::Id& selectedId)
	{
		Name ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = access.Get<const CType>(selectedId).name;
		}

		AST::Id lastId = selectedId;
		if (UI::BeginCombo(label.data(), ownerName.ToString().c_str()))
		{
			static ImGuiTextFilter filter;
			if (UI::IsWindowAppearing())
			{
				UI::SetKeyboardFocusHere();
			}
			UI::SetNextItemWidth(-FLT_MIN);
			filter.Draw("##Filter");

			auto nativeIds = ecs::ListAll<CType, CDeclNative>(access);
			auto structIds = ecs::ListAll<CType, CDeclStruct>(access);
			auto classIds  = ecs::ListAll<CType, CDeclClass>(access);
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

	bool InputLiteralValue(AST::Tree& ast, StringView label, AST::Id literalId)
	{
		return false;
	}
}    // namespace rift::Editor
