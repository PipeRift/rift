// Copyright 2015-2019 Piperift - All rights reserved

#include "StringPropertyWidget.h"
#include "imgui/imgui_stl.h"

#if WITH_EDITOR

void StringPropertyWidget::Tick(float)
{
	ImGui::PushID(GetName().ToString().c_str());
	ImGui::InputText(displayName.c_str(), *GetHandle()->GetValuePtr());
	ImGui::PopID();
}

#endif
