// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <Reflection/ClassType.h>
#include <Reflection/EnumType.h>
#include <Reflection/GetType.h>
#include <Reflection/Handles/PropertyHandle.h>
#include <Reflection/NativeType.h>
#include <Reflection/StructType.h>


namespace Rift::UI
{
	// label, data, type
	using CustomKeyValue = TFunction<void(StringView label, void*, Refl::Type*)>;


	void RegisterCustomInspection(Refl::Type* typeId, const CustomKeyValue& custom);
	template<typename T>
	void RegisterCustomInspection(const CustomKeyValue& custom)
	{
		RegisterCustomInspection(GetType<T>(), custom);
	}

	void DrawEnumValue(void* data, Refl::EnumType* type);
	void DrawNativeValue(void* data, Refl::NativeType* type);
	void DrawKeyValue(StringView label, void* data, Refl::Type* type);

	void InspectProperty(const Refl::PropertyHandle& property);
	void InspectProperties(void* container, Refl::DataType* type);

	inline void InspectStruct(Struct* data, Refl::StructType* type)
	{
		InspectProperties(data, type);
	}
	template<typename T>
	inline void InspectStruct(T* data) requires(Derived<T, Struct>)
	{
		InspectStruct(data, T::GetStaticType());
	}
	inline void InspectClass(Class* data, Refl::ClassType* type)
	{
		InspectProperties(data, type);
	}

	bool BeginInspectHeader(StringView label);
	void EndInspectHeader();

	bool BeginInspector(const char* name, v2 size = v2{0.f, 0.f});
	void EndInspector();

	void RegisterCoreKeyValueInspections();
}    // namespace Rift::UI
