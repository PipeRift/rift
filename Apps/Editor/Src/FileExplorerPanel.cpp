// Copyright 2015-2021 Piperift - All rights reserved

#include "FileExplorerPanel.h"
#include "ProjectEditor.h"

#include <imgui.h>
#include <imgui_internal.h>


void FileExplorerPanel::Draw()
{
	ImGui::SetNextWindowDockID(editor.fileExplorerDockID);
	if (ImGui::Begin("File Explorer", &bOpen))
	{
		if (ImGui::Button("I work!")) {}
	}
	ImGui::End();
}
