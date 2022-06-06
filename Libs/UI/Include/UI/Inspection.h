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


namespace rift::UI
{
	using namespace p;


	// label, data, type
	using CustomKeyValue = TFunction<void(StringView label, void*, Type*)>;


	void RegisterCustomInspection(Type* typeId, const CustomKeyValue& custom);
	template<typename T>
	void RegisterCustomInspection(const CustomKeyValue& custom)
	{
		RegisterCustomInspection(GetType<T>(), custom);
	}

	void DrawEnumValue(void* data, EnumType* type);
	void DrawNativeValue(void* data, NativeType* type);
	void DrawKeyValue(StringView label, void* data, Type* type);

	void InspectProperty(const PropertyHandle& property);
	void InspectProperties(void* container, DataType* type);

	inline void InspectStruct(Struct* data, StructType* type)
	{
		InspectProperties(data, type);
	}
	template<typename T>
	inline void InspectStruct(T* data) requires(Derived<T, p::Struct>)
	{
		InspectStruct(data, T::GetStaticType());
	}
	inline void InspectClass(Class* data, ClassType* type)
	{
		InspectProperties(data, type);
	}

	bool BeginInspectHeader(StringView label);
	void EndInspectHeader();

	bool BeginInspector(const char* name, v2 size = v2{0.f, 0.f});
	void EndInspector();

	void RegisterCoreKeyValueInspections();
}    // namespace rift::UI
