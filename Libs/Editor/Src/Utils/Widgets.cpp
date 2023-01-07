// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/Widgets.h"

#include <UI/UI.h>


namespace rift::Editor
{
	using namespace p::core;


	void ListTypesFromFilter(p::TAccessRef<AST::CNamespace> access, p::TArray<AST::Id> typeIds,
	    AST::Id& selectedId, ImGuiTextFilter& searchFilter)
	{
		for (AST::Id id : typeIds)
		{
			const auto& type      = access.Get<const AST::CNamespace>(id);
			const p::String& name = type.name.ToString();

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

	bool TypeCombo(p::TAccessRef<AST::CNamespace, AST::CType, AST::CDeclNative, AST::CDeclStruct,
	                   AST::CDeclClass>
	                   access,
	    p::StringView label, AST::Id& selectedId)
	{
		p::Name ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = access.Get<const AST::CNamespace>(selectedId).name;
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

			auto nativeIds = p::ecs::ListAll<AST::CType, AST::CDeclNative, AST::CNamespace>(access);
			auto structIds = p::ecs::ListAll<AST::CType, AST::CDeclStruct, AST::CNamespace>(access);
			auto classIds  = p::ecs::ListAll<AST::CType, AST::CDeclClass, AST::CNamespace>(access);
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
