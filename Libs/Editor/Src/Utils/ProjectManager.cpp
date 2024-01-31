// Copyright 2015-2023 Piperift - All rights reserved

#include "Editor.h"
#include "Utils/ElementsPanel.h"

#include <Pipe/Files/FileDialog.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace rift::Editor
{
	void DrawProjectManager(ast::Tree& ast)
	{
		// Center modal when appearing
		UI::SetNextWindowPos(UI::GetMainViewport()->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});

		v2 viewportSize = UI::GetMainViewport()->Size;
		v2 modalSize    = v2{600.f, 400.f};
		modalSize.x     = p::Min(modalSize.x, viewportSize.x - 20.f);
		modalSize.y     = p::Min(modalSize.y, viewportSize.y - 20.f);

		UI::SetNextWindowSize(modalSize, ImGuiCond_Always);

		if (UI::BeginPopupModal("Project Manager", nullptr,
		        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			UI::PushFont("WorkSans", UI::FontMode::Regular, 18.f);
			UI::Text("Projects");
			UI::PopFont();
			UI::Separator();
			UI::Spacing();

			if (UI::Button("Open", v2{-FLT_MIN, 0.0f}))
			{
				String folder =
				    files::SelectFolderDialog("Select project folder", p::GetCurrentPath());
				if (Editor::Get().OpenProject(folder))
				{
					UI::CloseCurrentPopup();
				}
				else
				{
					UI::AddNotification({UI::ToastType::Error, 1.f,
					    Strings::Format("Failed to open project at '{}'", p::ToString(folder))});
				}
			}
			UI::SetItemDefaultFocus();
			{
				UI::Text("Recent Projects");
				static const char* recentProjects[]{"One recent project"};
				static int selectedN = 0;
				UI::SetNextItemWidth(-FLT_MIN);
				if (UI::BeginListBox("##RecentProjects"))
				{
					for (int n = 0; n < IM_ARRAYSIZE(recentProjects); ++n)
					{
						const bool isSelected = (selectedN == n);
						if (UI::Selectable(recentProjects[n], isSelected))
						{
							selectedN = n;
						}

						// Set the initial focus when opening the combo (scrolling + keyboard
						// navigation focus)
						if (isSelected)
						{
							UI::SetItemDefaultFocus();
						}
					}
					UI::EndListBox();
				}
			}


			UI::Spacing();
			UI::Spacing();
			UI::Spacing();
			UI::Spacing();

			UI::PushFont("WorkSans", UI::FontMode::Regular, 18.f);
			UI::Text("New");
			UI::PopFont();
			UI::Separator();
			UI::Spacing();

			UI::AlignTextToFramePadding();
			UI::Text("Destination");
			UI::SameLine();
			static String folder;
			UI::PushItemWidth(-32.f);
			UI::InputText("##path", folder);
			UI::PopItemWidth();
			UI::SameLine();
			if (UI::Button("...", v2{24.f, 0.f}))
			{
				Path selectedFolder =
				    files::SelectFolderDialog("Select project folder", p::GetCurrentPath());
				folder = p::ToString(selectedFolder);
			}

			if (UI::Button("Create", v2{-FLT_MIN, 0.0f}))
			{
				if (Editor::Get().CreateProject(folder))
				{
					folder = "";
					UI::CloseCurrentPopup();
				}
				else
				{
					UI::AddNotification({UI::ToastType::Error, 1.f,
					    Strings::Format("Failed to create project at '{}'", folder)});
				}
			}

			UI::EndPopup();
		}
	}

	void OpenProjectManager()
	{
		UI::OpenPopup("Project Manager");
	}
}    // namespace rift::Editor
