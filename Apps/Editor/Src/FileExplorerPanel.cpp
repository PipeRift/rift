// Copyright 2015-2021 Piperift - All rights reserved

#include "FileExplorerPanel.h"
#include "ProjectEditor.h"

#include <imgui.h>
#include <imgui_internal.h>


void FileExplorerPanel::Draw()
{
	ImGui::SetNextWindowDockID(editor.fileExplorerDockID);
	if (ImGui::Begin(
			"File Explorer", &bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (!editor.HasValidProject())
		{
			ImGui::Text("No active project.");
			if (ImGui::Button("Open Project"))
			{
				editor.SetProject(Path("Project"));
			}
		}
		else
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Filter"))
				{
					static bool bClasses = true;
					static bool bStructs = true;
					static bool bFunctions = true;
					ImGui::Checkbox("Classes", &bClasses);
					ImGui::Checkbox("Structs", &bStructs);
					ImGui::Checkbox("Function Libraries", &bFunctions);
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			DrawList();
		}
	}
	ImGui::End();
}

void FileExplorerPanel::DrawList()
{
	if (bDirty)
	{
		CacheProjectFiles();
	}

	// Directories
	{
		ImGui::BeginChild("Files");
		if (ImGui::TreeNodeEx("Directories", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Selectable(
				"Project.rf", false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0));
			ImGui::TreePop();
		}
		for (const auto& asset : editor.project->GetAllTypeAssets())
		{
			ImGui::Selectable(asset.GetStrPath().c_str(), false, 0,
				ImVec2(ImGui::GetWindowContentRegionWidth(), 0));
		}
		ImGui::EndChild();
	}
}

void FileExplorerPanel::CacheProjectFiles()
{
	editor.project->ScanAssets();
	bDirty = false;
}