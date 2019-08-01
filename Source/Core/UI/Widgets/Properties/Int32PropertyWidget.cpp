// Copyright 2015-2019 Piperift - All rights reserved

#include "Int32PropertyWidget.h"


#if WITH_EDITOR

void Int32PropertyWidget::Tick(float)
{
	ImGui::PushID(GetName().ToString().c_str());

	ImGui::InputInt(displayName.c_str(), GetHandle()->GetValuePtr());

	ImGui::PopID();
}

#endif
