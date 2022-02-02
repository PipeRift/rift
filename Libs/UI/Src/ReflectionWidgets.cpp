// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/ReflectionWidgets.h"

#include <Reflection/GetType.h>
#include <Reflection/Static/StructType.h>


namespace Rift::UI
{
	static const char* currentInspector = nullptr;


	void DrawEnum(void* data, Refl::EnumType* type) {}
	void DrawNativeValue(void* data, Refl::NativeType* type)
	{
		static String label;
		label.clear();
		Strings::FormatTo(label, "##{}", sizet(data));

		if (type == GetType<bool>())
		{
			UI::Checkbox(label.c_str(), static_cast<bool*>(data));
		}
		else if (type == GetType<u8>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_U8, data);
		}
		else if (type == GetType<i32>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_S32, data);
		}
		else if (type == GetType<u32>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_U32, data);
		}
		else if (type == GetType<i64>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_S64, data);
		}
		else if (type == GetType<u64>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_U64, data);
		}
		else if (type == GetType<float>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_Float, data);
		}
		else if (type == GetType<double>())
		{
			UI::InputScalar(label.c_str(), ImGuiDataType_Double, data);
		}
		else if (type == GetType<String>())
		{
			UI::InputText(label.c_str(), *static_cast<String*>(data));
		}
		else if (type == GetType<Name>())
		{
			Name& name  = *static_cast<Name*>(data);
			String text = name.ToString();
			if (UI::InputText(label.c_str(), text))
			{
				name = Name{text};
			}
		}
	}
	void InspectProperty(void* container, Refl::Property* property)
	{
		UI::TableNextRow();
		UI::TableNextColumn();

		void* data = property->GetDataPtr(container);
		if (auto* structType = property->GetType()->AsStruct())
		{
			static String label;
			label.clear();
			Strings::FormatTo(label, "{}##{}", property->GetName(), container);
			UI::Selectable(label.c_str(), true,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);

			UI::TableNextColumn();

			UI::Indent(20.f);
			InspectProperties(data, structType);
			UI::Unindent(20.f);
		}
		else
		{
			UI::Text(property->GetName().ToString());

			UI::TableNextColumn();
			if (auto* nativeType = property->GetType()->AsNative())
			{
				DrawNativeValue(data, nativeType);
			}
		}
	}

	void InspectProperties(void* container, Refl::DataType* type)
	{
		if (!EnsureMsg(currentInspector,
		        "Make sure to call Begin/EndInspector around reflection widgets."))
		{
			return;
		}

		TArray<Refl::Property*> properties;
		type->GetProperties(properties);
		for (auto* prop : properties)
		{
			InspectProperty(container, prop);
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
			return false;
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
