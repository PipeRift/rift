// Copyright 2015-2021 Piperift - All rights reserved

#include "AssetEditor.h"

#include "Window.h"

#include <imgui.h>


void AssetEditor::Draw()
{
	if (ImGui::Begin("File", &bOpen))
	{
		ImGui::DockSpace(ImGui::GetID(this));
	}
	ImGui::End();
}
