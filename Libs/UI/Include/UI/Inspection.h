// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <Reflection/ClassType.h>
#include <Reflection/EnumType.h>
#include <Reflection/Handles/PropertyHandle.h>
#include <Reflection/NativeType.h>
#include <Reflection/StructType.h>


namespace Rift::UI
{
	void DrawEnumValue(void* data, Refl::EnumType* type);
	void DrawNativeValue(void* data, Refl::NativeType* type);

	void InspectProperty(const Refl::PropertyHandle& property);
	void InspectProperties(void* container, Refl::DataType* type);

	inline void InspectStruct(Struct* data, Refl::StructType* type)
	{
		InspectProperties(data, type);
	}
	inline void InspectClass(Class* data, Refl::ClassType* type)
	{
		InspectProperties(data, type);
	}

	bool BeginInspectHeader(const char* label);
	void EndInspectHeader();

	bool BeginInspector(const char* name, v2 size = v2{0.f, 0.f});
	void EndInspector();
}    // namespace Rift::UI
