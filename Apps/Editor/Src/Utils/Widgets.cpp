// Copyright 2015-2021 Piperift - All rights reserved

#include "Utils/Widgets.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CNativeDecl.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CType.h>
#include <UI/UI.h>


namespace Rift::Editor
{
	template<typename Filter>
	void ListTypesFromFilter(
	    Filter& filter, AST::Id& selectedId, bool& changed, ImGuiTextFilter& searchFilter)
	{
		for (AST::Id id : filter)
		{
			auto& type         = filter.Get<CType>(id);
			const String& name = type.name.ToString();

			if (!searchFilter.PassFilter(name.c_str(), name.c_str() + name.size()))
			{
				continue;
			}

			UI::PushID(u32(id));
			if (UI::Selectable(name.c_str(), id == selectedId))
			{
				selectedId = id;
				changed    = true;
			}
			UI::PopID();
		}
	}

	bool TypeCombo(AST::Tree& ast, StringView label, AST::Id& selectedId)
	{
		auto identifiers = ast.Filter<CType, CIdentifier>();

		Name typeName;
		if (!IsNone(selectedId))
		{
			auto& identifier = identifiers.Get<CIdentifier>(selectedId);
			typeName         = identifier.name;
		}

		bool changed = false;
		if (UI::BeginCombo(label.data(), typeName.ToString().c_str()))
		{
			static ImGuiTextFilter filter;
			if (UI::IsWindowAppearing())
			{
				UI::SetKeyboardFocusHere();
			}
			filter.Draw("##Filter");

			auto natives = ast.Filter<CType, CNativeDecl>();
			auto structs = ast.Filter<CType, CStructDecl>();
			auto classes = ast.Filter<CType, CClassDecl>();

			if (filter.IsActive())
			{
				ListTypesFromFilter(natives, selectedId, changed, filter);
				ListTypesFromFilter(structs, selectedId, changed, filter);
				ListTypesFromFilter(classes, selectedId, changed, filter);
			}
			else
			{
				ListTypesFromFilter(natives, selectedId, changed, filter);
				if (UI::TreeNode("Structs"))
				{
					ListTypesFromFilter(structs, selectedId, changed, filter);
					UI::TreePop();
				}
				if (UI::TreeNode("Classes"))
				{
					ListTypesFromFilter(classes, selectedId, changed, filter);
					UI::TreePop();
				}
			}
			UI::EndCombo();
		}

		return changed;
	}
}    // namespace Rift::Editor
