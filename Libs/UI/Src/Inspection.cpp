// Copyright 2015-2023 Piperift - All rights reserved

#include "UI/Inspection.h"

#include "imgui.h"

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
		if (UI::BeginCombo(label.c_str(), type->GetNameByIndex(currentIndex).AsString().data()))
		{
			for (i32 i = 0; i < type->Size(); ++i)
			{
				const bool isSelected = currentIndex == i;

				if (UI::Selectable(type->GetNameByIndex(i).AsString().data(), isSelected))
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
		else if (type == GetType<Tag>())
		{
			Tag& name = *static_cast<Tag*>(data);
			String text{name.AsString()};
			if (UI::InputText(label.c_str(), text))
			{
				name = Tag{text};
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
		if (auto* nativeType = Cast<NativeType>(type))
		{
			DrawNativeValue(data, nativeType);
		}
		else if (auto* enumType = Cast<EnumType>(type))
		{
			DrawEnumValue(data, enumType);
		}
	}

	void DrawArrayValue(const ArrayProperty& property, void* instance)
	{
		UI::Text(Strings::Format("{} items", property.GetSize(instance)));

		// Ignore indent on buttons
		const float widthAvailable =
		    ImGui::GetContentRegionAvail().x + UI::GetCurrentWindow()->DC.Indent.x;
		UI::SameLine(widthAvailable - 50.f);
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
	}

	void DrawArrayItemButtons(const ValueHandle& handle)
	{
		const float widthAvailable =
		    ImGui::GetContentRegionAvail().x + UI::GetCurrentWindow()->DC.Indent.x;
		UI::SameLine(widthAvailable - 50.f);
		UI::PushStyleCompact();
		static String label;
		label.clear();
		Strings::FormatTo(label, ICON_FA_TIMES "##removeItem_{}", handle.GetIndex());
		if (UI::Button(label.c_str(), v2(18.f, 18.f)))
		{
			handle.GetArrayProperty()->RemoveItem(handle.GetContainerPtr(), handle.GetIndex());
		}
		UI::PopStyleCompact();
	}

	void InspectProperty(const ValueHandle& handle)
	{
		auto* type = handle.GetType();
		if (!type)
		{
			return;
		}

		void* instance = handle.GetPtr();
		UI::PushID(instance);

		bool isLeaf = false;
		if (handle.IsArray())
		{
			isLeaf = handle.GetArrayProperty()->GetSize(instance) > 0;
		}
		else if (auto* custom = gCustomKeyValues.Find(type))
		{
			(*custom)(handle.GetDisplayName(), instance, type);
			if (handle.IsArrayItem())
			{
				DrawArrayItemButtons(handle);
			}
			UI::PopID();
			return;
		}
		else if (auto* structType = Cast<StructType>(type))
		{
			isLeaf = !structType->IsEmpty();
		}
		else
		{
			DrawKeyValue(handle.GetDisplayName(), instance, type);
			if (handle.IsArrayItem())
			{
				DrawArrayItemButtons(handle);
			}
			UI::PopID();
			return;
		}

		bool bOpen = BeginCategory(handle.GetDisplayName(), isLeaf);

		if (handle.IsArray())
		{
			UI::TableSetColumnIndex(1);
			DrawArrayValue(*handle.GetArrayProperty(), instance);
		}

		if (bOpen)
		{
			InspectChildrenProperties(handle);
			EndCategory();
		}
		UI::PopID();
	}

	void InspectChildrenProperties(const ValueHandle& handle)
	{
		if (!EnsureMsg(gCurrentInspector,
		        "Make sure to call Begin/EndInspector around reflection widgets."))
		{
			return;
		}

		const auto* type = handle.GetType();
		if (!type)
		{
			return;
		}

		if (auto* structType = Cast<StructType>(type))
		{
			void* instance = handle.GetPtr();
			TArray<Property*> properties;
			structType->GetProperties(properties);
			for (auto* prop : properties)
			{
				InspectProperty({instance, prop});
			}
		}
		else if (handle.IsArray())
		{
			auto* arrayProperty = handle.GetArrayProperty();
			void* instance      = handle.GetPtr();
			const i32 size      = arrayProperty->GetSize(instance);
			for (i32 i = 0; i < size; ++i)
			{
				InspectProperty({handle, i});
			}
		}
	}

	bool BeginCategory(StringView name, bool isLeaf)
	{
		UI::TableNextRow();
		UI::TableSetColumnIndex(0);
		UI::PushHeaderColor(UI::GetNeutralColor(1));
		UI::AlignTextToFramePadding();
		const ImGuiTreeNodeFlags flags =
		    ImGuiTreeNodeFlags_AllowItemOverlap | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0);

		bool bOpen = UI::TreeNodeEx(name.data(), ImGuiTreeNodeFlags_AllowItemOverlap);
		UI::PopHeaderColor();
		return bOpen;
	}
	void EndCategory()
	{
		UI::TreePop();
	}

	bool BeginInspector(const char* label, v2 size)
	{
		if (!EnsureMsg(!gCurrentInspector,
		        "Called BeginInspector() twice without calling EndInspector() first."))
		{
			return false;
		}

		const ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp
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
			UI::SetNextItemWidth(p::Min(300.f, UI::GetContentRegionAvail().x));
			String str = ToString(*path);
			if (UI::InputText("##value", str))
			{
				*path = p::ToSTDPath(str);
			}
		});
	}
}    // namespace rift::UI
