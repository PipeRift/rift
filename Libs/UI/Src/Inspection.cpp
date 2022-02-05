// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/Inspection.h"

#include <Reflection/GetType.h>
#include <Reflection/StructType.h>


namespace Rift::UI
{
	static const char* currentInspector = nullptr;


	void DrawEnumValue(void* data, Refl::EnumType* type)
	{
		static String label;
		label.clear();
		Strings::FormatTo(label, "##{}", sizet(data));

		const i32 currentIndex = type->GetIndexFromValue(data);
		if (UI::BeginCombo(label.c_str(), type->GetNameByIndex(currentIndex).ToString().c_str()))
		{
			for (i32 i = 0; i < type->Size(); ++i)
			{
				const bool isSelected = currentIndex == i;

				if (UI::Selectable(type->GetNameByIndex(i).ToString().c_str(), isSelected))
				{
					type->SetValueFromIndex(data, i);
				}

				// Set the initial focus when opening the combo
				if (isSelected)
				{
					UI::SetItemDefaultFocus();
				}
			}
			UI::EndCombo();
		}
	}

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
		else if (type == GetType<v2>())
		{
			UI::InputFloat2(label.c_str(), static_cast<float*>(data));
		}
		else if (type == GetType<v3>())
		{
			UI::InputFloat3(label.c_str(), static_cast<float*>(data));
		}
	}
	void InspectProperty(const Refl::PropertyHandle& handle)
	{
		UI::TableNextRow();

		StringView name = handle.GetDisplayName();

		if (auto* structType = handle.GetType() ? handle.GetType()->AsStruct() : nullptr)
		{
			UI::TableSetColumnIndex(0);
			if (BeginInspectHeader(name.data()))
			{
				InspectProperties(handle.GetPtr(), structType);
				EndInspectHeader();
			}
		}
		else
		{
			UI::TableSetColumnIndex(0);
			UI::AlignTextToFramePadding();
			UI::Text(name);

			UI::TableSetColumnIndex(1);
			if (auto* nativeType = handle.GetType() ? handle.GetType()->AsNative() : nullptr)
			{
				DrawNativeValue(handle.GetPtr(), nativeType);
			}
			else if (auto* enumType = handle.GetType() ? handle.GetType()->AsEnum() : nullptr)
			{
				DrawEnumValue(handle.GetPtr(), enumType);
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

		UI::PushID(container);

		TArray<Refl::Property*> properties;
		type->GetProperties(properties);
		for (auto* prop : properties)
		{
			InspectProperty(Refl::PropertyHandle{*prop, container});
		}

		UI::PopID();
	}

	bool BeginInspectHeader(const char* label)
	{
		Style::PushHeaderColor(Style::GetNeutralColor(1));

		UI::AlignTextToFramePadding();
		bool isOpen = UI::CollapsingHeader(
		    label, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap);
		Style::PopHeaderColor();

		if (isOpen)
		{
			UI::Indent(20.f);
		}
		return isOpen;
	}

	void EndInspectHeader()
	{
		UI::Unindent(20.f);
	}

	bool BeginInspector(const char* label, v2 size)
	{
		if (!EnsureMsg(!currentInspector,
		        "Called BeginInspector() twice without calling EndInspector() first."))
		{
			return false;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_SizingStretchProp
		                                   | ImGuiTableFlags_PadOuterX;
		if (UI::BeginTable(label, 2, flags, size))
		{
			currentInspector = label;
			UI::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			UI::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.5f);
		}

		return true;
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
