// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/Inspection.h"

#include <IconsFontAwesome5.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/STDFileSystem.h>
#include <Pipe/Reflect/GetType.h>
#include <Pipe/Reflect/StructType.h>


namespace rift::UI
{
	static const char* gCurrentInspector = nullptr;

	static TMap<Type*, CustomKeyValue> gCustomKeyValues;


	void RegisterCustomInspection(Type* typeId, const CustomKeyValue& custom)
	{
		if (custom)
		{
			gCustomKeyValues.Insert(typeId, custom);
		}
	}

	void DrawEnumValue(void* data, EnumType* type)
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

	void DrawNativeValue(void* data, NativeType* type)
	{
		static String label;
		label.clear();
		Strings::FormatTo(label, "##{}", sizet(data));

		if (type == GetType<bool>())
		{
			UI::Checkbox(label.c_str(), static_cast<bool*>(data));
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
		else if (type == GetType<v2>())
		{
			UI::InputFloat2(label.c_str(), static_cast<float*>(data));
		}
		else if (type == GetType<v3>())
		{
			UI::InputFloat3(label.c_str(), static_cast<float*>(data));
		}
	}

	void DrawKeyValue(StringView label, void* data, Type* type)
	{
		UI::TableNextRow();
		UI::TableSetColumnIndex(0);
		UI::AlignTextToFramePadding();
		UI::Text(label);
		UI::TableSetColumnIndex(1);
		if (auto* nativeType = type->AsNative())
		{
			DrawNativeValue(data, nativeType);
		}
		else if (auto* enumType = type->AsEnum())
		{
			DrawEnumValue(data, enumType);
		}
	}

	void DrawArrayValue(bool open, const ArrayProperty& property, void* instance)
	{
		UI::Text(Strings::Format("{} items", property.GetSize(instance)));
		if (open)    // NOTE: Prevents buttons from being affected by indent
		{
			UI::Unindent(20.f);
		}
		UI::SameLine(ImGui::GetContentRegionAvailWidth() - 50.f);
		UI::PushStyleCompact();
		if (UI::Button(ICON_FA_PLUS "##AddItem", v2(16.f, 18.f)))
		{
			property.AddItem(instance, nullptr);
		}
		UI::SameLine();
		if (UI::Button(ICON_FA_TRASH_ALT "##Empty", v2(16.f, 18.f)))
		{
			property.Clear(instance);
		}
		UI::PopStyleCompact();
		if (open)
		{
			UI::Indent(20.f);
		}
	}

	void DrawArrayItemButtons(const ArrayProperty& property, void* instance, i32 index)
	{
		UI::SameLine(ImGui::GetContentRegionAvailWidth() - 20.f);
		UI::PushStyleCompact();
		static String label;
		label.clear();
		Strings::FormatTo(label, ICON_FA_TIMES "##removeItem_{}", index);
		if (UI::Button(label.c_str(), v2(18.f, 18.f)))
		{
			property.RemoveItem(instance, index);
		}
		UI::PopStyleCompact();
	}

	void InspectArrayProperty(const ArrayProperty& property, void* instance)
	{
		const bool open = BeginInspectHeader(property.GetDisplayName().data());
		UI::TableSetColumnIndex(1);
		DrawArrayValue(open, property, instance);
		if (open)
		{
			const i32 size = property.GetSize(instance);
			auto* type     = property.GetType();
			static String label;
			if (auto* structType = type->AsStruct())
			{
				for (i32 i = 0; i < size; ++i)
				{
					label.clear();
					Strings::FormatTo(label, "{}", i);
					const bool open = BeginInspectHeader(label);
					if (open)
					{
						UI::Unindent(20.f);
					}
					UI::TableSetColumnIndex(1);
					DrawArrayItemButtons(property, instance, i);
					if (open)
					{
						UI::Indent(20.f);
						InspectProperties(property.GetItem(instance, i), structType);
						EndInspectHeader();
					}
				}
			}
			else if (auto* custom = gCustomKeyValues.Find(type))
			{
				for (i32 i = 0; i < size; ++i)
				{
					label.clear();
					Strings::FormatTo(label, "{}", i);
					(*custom)(label, property.GetItem(instance, i), type);
					DrawArrayItemButtons(property, instance, i);
				}
			}
			else
			{
				for (i32 i = 0; i < size; ++i)
				{
					label.clear();
					Strings::FormatTo(label, "{}", i);
					DrawKeyValue(label, property.GetItem(instance, i), type);
					DrawArrayItemButtons(property, instance, i);
				}
			}
			EndInspectHeader();
		}
	}

	void InspectProperty(const PropertyHandle& handle)
	{
		auto* type = handle.GetType();
		if (!type)
		{
			return;
		}

		void* instance = handle.GetPtr();
		UI::PushID(instance);
		if (auto* arrayProperty = handle.GetArrayProperty())
		{
			InspectArrayProperty(*arrayProperty, instance);
		}
		else if (auto* custom = gCustomKeyValues.Find(type))
		{
			(*custom)(handle.GetDisplayName(), instance, type);
		}
		else if (auto* structType = type->AsStruct())
		{
			if (BeginInspectHeader(handle.GetDisplayName()))
			{
				InspectProperties(instance, structType);
				EndInspectHeader();
			}
		}
		else
		{
			DrawKeyValue(handle.GetDisplayName(), instance, type);
		}
		UI::PopID();
	}

	void InspectProperties(void* container, DataType* type)
	{
		if (!EnsureMsg(gCurrentInspector,
		        "Make sure to call Begin/EndInspector around reflection widgets."))
		{
			return;
		}

		UI::PushID(container);

		TArray<Property*> properties;
		type->GetProperties(properties);
		for (auto* prop : properties)
		{
			InspectProperty(PropertyHandle{*prop, container});
		}

		UI::PopID();
	}

	bool BeginInspectHeader(StringView label)
	{
		UI::TableNextRow();
		UI::TableSetColumnIndex(0);
		UI::PushHeaderColor(UI::GetNeutralColor(1));

		UI::AlignTextToFramePadding();
		bool isOpen = UI::CollapsingHeader(
		    label.data(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap);
		UI::PopHeaderColor();

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
		if (!EnsureMsg(!gCurrentInspector,
		        "Called BeginInspector() twice without calling EndInspector() first."))
		{
			return false;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_SizingStretchProp
		                                   | ImGuiTableFlags_PadOuterX;
		if (UI::BeginTable(label, 2, flags, size))
		{
			gCurrentInspector = label;
			UI::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			UI::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1.f);
			return true;
		}
		return false;
	}

	void EndInspector()
	{
		if (!EnsureMsg(gCurrentInspector, "Called EndInspector() but no inspector was drawing."))
		{
			return;
		}
		UI::EndTable();
		gCurrentInspector = nullptr;
	}


	bool DrawColorKeyValue(StringView label, LinearColor& color, ImGuiColorEditFlags flags)
	{
		auto* data =
		    reinterpret_cast<float*>(&color);    // LinearColor* can be interpreted as float*
		UI::TableNextRow();
		UI::TableSetColumnIndex(0);
		UI::AlignTextToFramePadding();
		UI::Text(label);
		UI::TableSetColumnIndex(1);
		return UI::ColorEdit4("##value", data, flags | ImGuiColorEditFlags_AlphaBar);
	}

	void RegisterCoreKeyValueInspections()
	{
		UI::RegisterCustomInspection<LinearColor>([](StringView label, void* data, Type* type) {
			DrawColorKeyValue(label, *reinterpret_cast<LinearColor*>(data),
			    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreviewHalf);
		});

		UI::RegisterCustomInspection<HSVColor>([](StringView label, void* data, Type* type) {
			auto* color = reinterpret_cast<HSVColor*>(data);
			LinearColor lColor{*color};
			if (DrawColorKeyValue(label, lColor,
			        ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayHSV
			            | ImGuiColorEditFlags_AlphaPreviewHalf))
			{
				*color = HSVColor{lColor};
			}
		});

		UI::RegisterCustomInspection<Color>([](StringView label, void* data, Type* type) {
			auto* color = reinterpret_cast<Color*>(data);
			LinearColor lColor{*color};
			if (DrawColorKeyValue(label, lColor,
			        ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB
			            | ImGuiColorEditFlags_AlphaPreviewHalf))
			{
				*color = Color{lColor};
			}
		});

		UI::RegisterCustomInspection<Path>([](StringView label, void* data, Type* type) {
			auto* path = reinterpret_cast<Path*>(data);
			UI::TableNextRow();
			UI::TableSetColumnIndex(0);
			UI::AlignTextToFramePadding();
			UI::Text(label);
			UI::TableSetColumnIndex(1);
			UI::SetNextItemWidth(math::Min(300.f, UI::GetContentRegionAvail().x));
			String str = ToString(*path);
			if (UI::InputText("##value", str))
			{
				*path = p::ToPath(str);
			}
		});
	}
}    // namespace rift::UI
