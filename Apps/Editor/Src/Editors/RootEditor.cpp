// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/RootEditor.h"

#include <Files/FileDialog.h>
#include <imgui.h>


bool RootEditor::OpenProject(Path path)
{
	if (path.empty())
	{
		return false;
	}

	if (projectEditor)
	{
		CloseProject();
	}

	projectEditor = Create<ProjectEditor>();
	projectEditor->SetProject(path);

	if (!projectEditor->HasProject())
	{
		// TODO: Show loading error popup
		projectEditor.Release();
		return false;
	}
	return true;
}

void RootEditor::CloseProject()
{
	projectEditor.Release();
}

void RootEditor::Draw()
{
	if (projectEditor && projectEditor->HasProject())
	{
		projectEditor->Draw();
	}
	else
	{
		ImGui::OpenPopup("Project Picker");
	}
	DrawProjectPickerPopup();
}

void RootEditor::DrawProjectPickerPopup()
{
	if (ImGui::BeginPopupModal("Project Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Open Project..."))
		{
			Path folder = Dialogs::SelectFolder("Select project folder", FileSystem::GetCurrent());
			OpenProject(folder);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::Separator();
		ImGui::Text("Recent Projects");
		static const char* recentProjects[]{};
		static int selected = 0;
		if (ImGui::BeginListBox("##RecentProjects"))
		{
			for (int n = 0; n < IM_ARRAYSIZE(recentProjects); ++n)
			{
				const bool is_selected = (selected == n);
				if (ImGui::Selectable(recentProjects[n], is_selected))
				{
					selected = n;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation
				// focus)
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndPopup();
	}
}
