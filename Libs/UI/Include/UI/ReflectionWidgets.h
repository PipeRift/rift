// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <Reflection/Static/DataType.h>


namespace Rift::UI
{
	void DrawEnum(void* data, Refl::EnumType* type);
	void DrawNative(void* data, Refl::NativeType* type);

	void InspectProperty(void* container, Refl::Property* type);
	void InspectProperties(void* container, Refl::DataType* type);

	void InspectStruct(Struct* data, Refl::StructType* type);
	void InspectClass(Class* data, Refl::ClassType* type);

	bool BeginInspector(const char* name, v2 size = v2{0.f, UI::GetContentRegionAvail().y});
	void EndInspector();
}    // namespace Rift::UI
