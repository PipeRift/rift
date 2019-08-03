// Copyright 2015-2019 Piperift - All rights reserved

#include "Widget.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


void Widget::OnTick(float deltaTime)
{
	if (bBuilt)
	{
		const bool bDisabledNow = bDisabled;
		if (bDisabledNow)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		Tick(deltaTime);
		if (bDisabledNow)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
}
