// Copyright 2015-2019 Piperift - All rights reserved

#include "FloatPropertyWidget.h"
#include "Core/Math/Math.h"

#if WITH_EDITOR

void FloatPropertyWidget::Tick(float)
{
	ImGui::PushID(GetName().ToString().c_str());
	ImGui::InputFloat(displayName.c_str(), GetHandle()->GetValuePtr());
	ImGui::PopID();
}

#endif
