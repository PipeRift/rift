// Copyright 2015-2019 Piperift - All rights reserved

#include "UInt8PropertyWidget.h"
#include "Core/Math/Math.h"

#if WITH_EDITOR

void UInt8PropertyWidget::Tick(float)
{
	ImGui::PushID(GetName().ToString().c_str());

	i32 value = *GetHandle()->GetValuePtr();
	ImGui::InputInt(displayName.c_str(), &value);

	value = Math::Clamp(value, 0, 255);
	if (value != *GetHandle()->GetValuePtr())
	{
		GetHandle()->SetValue((u8)value);
	}

	ImGui::PopID();
}

#endif
