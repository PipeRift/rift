// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <Pipe/Reflect/ClassType.h>
#include <Pipe/Reflect/EnumType.h>
#include <Pipe/Reflect/GetType.h>
#include <Pipe/Reflect/Handles/PropertyHandle.h>
#include <Pipe/Reflect/NativeType.h>
#include <Pipe/Reflect/StructType.h>


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

	void InspectProperty(const ValueHandle& handle);
	void InspectChildrenProperties(const ValueHandle& handle);

	inline void InspectStruct(Struct* data, StructType* type)
	{
		// InspectChildrenProperties(data, type);
	}
	template<typename T>
	inline void InspectStruct(T* data)
	    requires(Derived<T, p::Struct>)
	{
		InspectStruct(data, T::GetStaticType());
	}
	inline void InspectClass(Class* data, ClassType* type)
	{
		// InspectChildrenProperties(data, type);
	}

	bool BeginCategory(StringView name, bool isLeaf);
	void EndCategory();

	bool BeginInspector(const char* name, v2 size = v2{0.f, 0.f});
	void EndInspector();

	void RegisterCoreKeyValueInspections();
}    // namespace rift::UI
