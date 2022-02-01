// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/ReflectionWidgets.h"


namespace Rift::UI
{
	static const char* currentInspector = nullptr;


	void DrawEnum(void* data, Refl::EnumType* type) {}
	void DrawNative(void* data, Refl::NativeType* type) {}
	void InspectProperty(void* container, Refl::Property* property)
	{
		UI::TableNextRow();
		UI::TableNextColumn();
		UI::Text(property->GetName().ToString());

		UI::TableNextColumn();
	}

	void InspectProperties(void* container, Refl::DataType* type)
	{
		if (!EnsureMsg(currentInspector,
		        "Make sure to call Begin/EndInspector around reflection widgets."))
		{
			return;
		}

		Refl::PropertyMap properties;
		type->GetAllProperties(properties);
		for (auto& it : properties)
		{
			InspectProperty(container, it.second);
		}
	}
	void InspectStruct(Struct* data, Refl::StructType* type) {}
	void InspectClass(Class* data, Refl::ClassType* type) {}

	void BeginInspectorHeader(const char* name, void* data, Refl::DataType* type, v2 size) {}
	void EndInspectorHeader(const char* name, void* data, Refl::DataType* type, v2 size) {}

	bool BeginInspector(const char* name, v2 size)
	{
		if (!EnsureMsg(!currentInspector,
		        "Called BeginInspector() twice without calling EndInspector() first."))
		{
			return;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_SizingStretchProp
		                                   | ImGuiTableFlags_BordersInnerV;
		if (UI::BeginTable(name, 2, flags, size))
		{
			currentInspector = name;
			UI::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			UI::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.5f);

			UI::TableHeadersRow();

			return true;
		}
		return false;
	}

	void EndInspector()
	{
		if (!EnsureMsg(currentInspector, "Called EndInspector() but no inspector was drawing."))
		{
			return;
		}
		UI::EndTable();
		currentInspector = nullptr;
	}
}    // namespace Rift::UI
