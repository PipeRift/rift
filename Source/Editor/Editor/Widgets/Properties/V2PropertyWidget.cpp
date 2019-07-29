// Copyright 2015-2019 Piperift - All rights reserved

#include "V2PropertyWidget.h"
#include "Core/Math/Math.h"
#include "Core/Reflection/ReflectionTags.h"

#if WITH_EDITOR

void V2PropertyWidget::Tick(float)
{
	ImGuiInputTextFlags flags = 0;
	if (!prop->HasTag(DetailsEdit))
		flags |= ImGuiInputTextFlags_ReadOnly;

	ImGui::PushID(GetName().ToString().c_str());

	v2* const val = GetHandle()->GetValuePtr();
	ImGui::InputFloat2(displayName.c_str(), val->Data(), "%g", flags);

	ImGui::PopID();
}

#endif
