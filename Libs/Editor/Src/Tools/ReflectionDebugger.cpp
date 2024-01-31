// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/ReflectionDebugger.h"

#include <AST/Components/Statements.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Reflect/TypeRegistry.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


namespace rift::Editor
{
	ReflectionDebugger::ReflectionDebugger() {}

	void ReflectionDebugger::Draw()
	{
		if (!open)
		{
			return;
		}

		const auto& registry = TypeRegistry::Get();

		UI::Begin("Reflection", &open);

		if (UI::BeginPopup("Filter"))
		{
			UI::CheckboxFlags("Native", (u32*)&categoryFilter, u32(TypeCategory::Native));
			UI::CheckboxFlags("Enum", (u32*)&categoryFilter, u32(TypeCategory::Enum));
			UI::CheckboxFlags("Class", (u32*)&categoryFilter, u32(TypeCategory::Class));
			UI::CheckboxFlags("Struct", (u32*)&categoryFilter, u32(TypeCategory::Struct));
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
			UI::TableSetupColumn("Category");
			UI::TableSetupColumn("Name");
			UI::TableSetupColumn("Parent");
			UI::TableHeadersRow();

			for (auto it : registry)
			{
				DrawType(it.second);
			}
			UI::EndTable();
		}
		UI::EndChild();

		UI::End();
	}

	void ReflectionDebugger::DrawType(Type* type)
	{
		if (!HasAllFlags(categoryFilter, type->GetCategory()))
		{
			return;
		}

		static String idText;
		idText.clear();
		Strings::FormatTo(idText, "{}", type->GetId());

		StringView name = type->GetName();
		if (!filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
		    && !filter.PassFilter(name.data(), name.data() + name.size()))
		{
			return;
		}

		UI::TableNextRow();

		UI::TableSetColumnIndex(0);    // Id
		UI::Text(idText);

		UI::TableSetColumnIndex(1);    // Category
		static String categories;
		categories.clear();
		GetEnumFlagName<TypeCategory>(type->GetCategory(), categories);
		UI::Text(categories);

		UI::TableSetColumnIndex(2);    // Name
		UI::Text(name);

		if (const auto* dataType = Cast<DataType>(type))
		{
			if (const DataType* parent = dataType->GetParent())
			{
				UI::TableSetColumnIndex(3);    // Parent
				UI::Text(parent->GetName());
			}
		}
	}
}    // namespace rift::Editor
