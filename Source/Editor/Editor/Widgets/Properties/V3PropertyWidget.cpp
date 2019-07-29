// Copyright 2015-2019 Piperift - All rights reserved

#include "V3PropertyWidget.h"
#include "Core/Math/Math.h"
#include "Core/Reflection/ReflectionTags.h"

#if WITH_EDITOR


void V3PropertyWidget::Tick(float)
{
	ImGuiInputTextFlags flags = 0;
	if (!prop->HasTag(DetailsEdit))
		flags |= ImGuiInputTextFlags_ReadOnly;

	ImGui::PushID(GetName().ToString().c_str());

	v3* const val = GetHandle()->GetValuePtr();
	ImGui::InputFloat3(displayName.c_str(), val->Data(), "%g", flags);

	ImGui::PopID();
}

#endif
