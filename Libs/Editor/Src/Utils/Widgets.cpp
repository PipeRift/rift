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
			const auto& type   = access.Get<const AST::CNamespace>(id);
			p::StringView name = type.name.AsString();

			if (!searchFilter.PassFilter(name.data(), name.data() + name.size()))
			{
				continue;
			}

			UI::PushID(u32(id));
			if (UI::Selectable(name.data(), id == selectedId))
			{
				selectedId = id;
			}
			UI::PopID();
		}
	}

	bool TypeCombo(p::TAccessRef<AST::CNamespace, AST::CDeclType, AST::CDeclNative,
	                   AST::CDeclStruct, AST::CDeclClass>
	                   access,
	    p::StringView label, AST::Id& selectedId)
	{
		p::Tag ownerName;
		if (!IsNone(selectedId))
		{
			ownerName = access.Get<const AST::CNamespace>(selectedId).name;
		}

		AST::Id lastId = selectedId;
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
			    p::FindAllIdsWith<AST::CDeclType, AST::CDeclNative, AST::CNamespace>(access);
			auto structIds =
			    p::FindAllIdsWith<AST::CDeclType, AST::CDeclStruct, AST::CNamespace>(access);
			auto classIds =
			    p::FindAllIdsWith<AST::CDeclType, AST::CDeclClass, AST::CNamespace>(access);
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

	bool InputLiteralValue(AST::Tree& ast, p::StringView label, AST::Id literalId)
	{
		return false;
	}
}    // namespace rift::Editor
