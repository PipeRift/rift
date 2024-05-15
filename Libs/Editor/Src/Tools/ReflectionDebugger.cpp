// Copyright 2015-2024 Piperift - All rights reserved

#include "Tools/ReflectionDebugger.h"

#include "PipeReflect.h"

#include <AST/Components/Statements.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


namespace rift::editor
{
	ReflectionDebugger::ReflectionDebugger() {}

	void ReflectionDebugger::Draw()
	{
		if (!open)
		{
			return;
		}

		UI::Begin("Reflection", &open);

		if (UI::BeginPopup("Filter"))
		{
			UI::CheckboxFlags("Native", &typeFlagsFilter, u64(p::TF_Native));
			UI::CheckboxFlags("Enum", &typeFlagsFilter, u64(p::TF_Enum));
			UI::CheckboxFlags("Struct", &typeFlagsFilter, u64(p::TF_Struct));
			UI::CheckboxFlags("Object", &typeFlagsFilter, u64(p::TF_Object));
			UI::CheckboxFlags("Container", &typeFlagsFilter, u64(p::TF_Container));
			UI::EndPopup();
		}
		if (UI::Button("Filter"))
		{
			UI::OpenPopup("Filter");
		}

		UI::SameLine();
		filter.Draw("##Filter", -100.0f);


		static ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
		                             | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingStretchProp;
		ImGui::BeginChild("typesTableChild", ImVec2(0.f, UI::GetContentRegionAvail().y));
		if (UI::BeginTable("typesTable", 4, flags))
		{
			UI::TableSetupColumn("Id", ImGuiTableColumnFlags_IndentEnable);
			UI::TableSetupColumn("Name");
			UI::TableSetupColumn("Flags");
			UI::TableSetupColumn("Parent");
			UI::TableHeadersRow();

			for (p::TypeId type : p::GetRegisteredTypeIds())
			{
				DrawType(type);
			}
			UI::EndTable();
		}
		UI::EndChild();

		UI::End();
	}

	void ReflectionDebugger::DrawType(TypeId type)
	{
		if (!HasAnyTypeFlags(type, typeFlagsFilter))
		{
			return;
		}

		static String idText;
		idText.clear();
		Strings::FormatTo(idText, "{}", type);

		StringView rawName = p::GetTypeName(type);
		if (!filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
		    && !filter.PassFilter(rawName.data(), rawName.data() + rawName.size()))
		{
			return;
		}

		UI::TableNextRow();

		UI::TableSetColumnIndex(0);    // Id
		UI::Text(idText);

		UI::TableSetColumnIndex(1);    // Name
		StringView ns;
		StringView name = p::RemoveNamespace(rawName, ns);
		UI::PushStyleCompact();
		UI::PushTextColor(UI::GetNeutralTextColor(1).Shade(0.3f));
		UI::Text(ns);
		UI::PopTextColor();
		UI::PopStyleCompact();
		UI::SameLine(0, 10.f);
		UI::Text(name);

		UI::TableSetColumnIndex(2);    // Flags
		static String flags;
		flags.clear();
		GetEnumFlagName<TypeFlags_>(TypeFlags_(GetTypeFlags(type)), flags);
		UI::Text(flags);

		TypeId parentId = p::GetTypeParent(type);
		if (parentId.IsValid())
		{
			UI::TableSetColumnIndex(3);    // Parent
			rawName = p::GetTypeName(parentId);
			name    = p::RemoveNamespace(rawName, ns);
			UI::PushStyleCompact();
			UI::PushTextColor(UI::GetNeutralTextColor(1).Shade(0.3f));
			UI::Text(ns);
			UI::PopTextColor();
			UI::PopStyleCompact();
			UI::SameLine(0, 10.f);
			UI::Text(name);
		}
	}
}    // namespace rift::editor
