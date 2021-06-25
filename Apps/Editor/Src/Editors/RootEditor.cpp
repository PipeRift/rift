// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/RootEditor.h"

#include <Files/FileDialog.h>
#include <RiftContext.h>
#include <UI/UI.h>


namespace Rift
{
	void RootEditor::Tick() {}

	void RootEditor::Draw()
	{
		if (GetContext<RiftContext>()->HasProject())
		{
			projectEditor->Draw();
		}
		else
		{
			UI::OpenPopup("Project Picker");
		}

		DrawMenuBar();
		DrawProjectPickerPopup();

		memoryDebugger.Draw();

#if BUILD_DEBUG
		if (showDemo)
		{
			UI::ShowDemoWindow(&showDemo);
		}
#endif
	}

	void RootEditor::DrawMenuBar()
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("Views"))
			{
				UI::MenuItem("Memory", nullptr, &memoryDebugger.open);
				UI::EndMenu();
			}

#if BUILD_DEBUG
			UI::MenuItem("Demo", nullptr, &showDemo);
#endif

			UI::EndMainMenuBar();
		}
	}

	void RootEditor::DrawProjectPickerPopup()
	{
		// Center modal when appearing
		UI::SetNextWindowPos(
		    UI::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (UI::BeginPopupModal("Project Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (UI::Button("Open Project..."))
			{
				Path folder = Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());

				if (GetContext<RiftContext>()->OpenProject(folder))
				{
					UI::CloseCurrentPopup();
				}
			}
			UI::SetItemDefaultFocus();
			UI::Separator();
			UI::Text("Recent Projects");
			static const char* recentProjects[]{"One recent project"};
			static int selected = 0;
			if (UI::BeginListBox("##RecentProjects"))
			{
				for (int n = 0; n < IM_ARRAYSIZE(recentProjects); ++n)
				{
					const bool is_selected = (selected == n);
					if (UI::Selectable(recentProjects[n], is_selected))
					{
						selected = n;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation
					// focus)
					if (is_selected)
					{
						UI::SetItemDefaultFocus();
					}
				}
				UI::EndListBox();
			}
			UI::EndPopup();
		}
	}
}    // namespace Rift
