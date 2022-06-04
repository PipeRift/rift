// Copyright 2015-2022 Piperift - All rights reserved

#include "Editor.h"
#include "Utils/Properties.h"

#include <Files/FileDialog.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace Rift
{
	void DrawProjectManager(AST::Tree& ast)
	{
		// Center modal when appearing
		UI::SetNextWindowPos(UI::GetMainViewport()->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});

		v2 viewportSize = UI::GetMainViewport()->Size;
		v2 modalSize    = v2{600.f, 400.f};
		modalSize.x     = Math::Min(modalSize.x, viewportSize.x - 20.f);
		modalSize.y     = Math::Min(modalSize.y, viewportSize.y - 20.f);

		UI::SetNextWindowSize(modalSize, ImGuiCond_Always);

		if (UI::BeginPopupModal("Project Manager", nullptr,
		        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			Style::PushFont("WorkSans", Style::FontMode::Regular, 18.f);
			UI::Text("Projects");
			Style::PopFont();
			UI::Separator();
			UI::Spacing();

			if (UI::Button("Open", v2{-FLT_MIN, 0.0f}))
			{
				Path folder =
				    Files::SelectFolderDialog("Select project folder", Pipe::GetCurrentPath());
				if (Editor::Get().OpenProject(folder))
				{
					UI::CloseCurrentPopup();
				}
				else
				{
					UI::AddNotification({UI::ToastType::Error, 1.f,
					    Strings::Format("Failed to open project at '{}'", Pipe::ToString(folder))});
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

			Style::PushFont("WorkSans", Style::FontMode::Regular, 18.f);
			UI::Text("New");
			Style::PopFont();
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
				    Files::SelectFolderDialog("Select project folder", Pipe::GetCurrentPath());
				folder = Pipe::ToString(selectedFolder);
			}

			if (UI::Button("Create", v2{-FLT_MIN, 0.0f}))
			{
				if (Editor::Get().CreateProject(FromString(folder)))
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
}    // namespace Rift
