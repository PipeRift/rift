// Copyright 2015-2021 Piperift - All rights reserved

#include "FileExplorerPanel.h"
#include "ProjectEditor.h"

#include <imgui.h>
#include <imgui_internal.h>


void FileExplorerPanel::Draw()
{
	ImGui::SetNextWindowDockID(editor.fileExplorerDockID);
	if (ImGui::Begin("File Explorer", &bOpen, ImGuiWindowFlags_NoCollapse))
	{
		if (!editor.project.IsValid())
		{
			ImGui::Text("No active project.");
			if (ImGui::Button("Open Project")) {}
		}
		else
		{
			if (ImGui::Button("List all files and folders here!")) {}
		}
	}
	ImGui::End();
}
