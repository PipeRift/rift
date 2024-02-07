// Copyright 2015-2023 Piperift - All rights reserved

#include "Editor.h"
#include "Utils/ElementsPanel.h"

#include <Pipe/Files/FileDialog.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace rift::Editor
{
	void TextCentered(const char* text)
	{
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth   = ImGui::CalcTextSize(text).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text);
	}

	void DrawProjectManager(ast::Tree& ast)
	{
		// Center modal when appearing
		UI::SetNextWindowPos(UI::GetMainViewport()->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});

		p::v2 viewportSize = UI::GetMainViewport()->Size;
		p::v2 modalSize    = p::v2{600.f, 0.f};
		modalSize.x        = p::Min(modalSize.x, viewportSize.x - 20.f);
		modalSize.y        = p::Min(modalSize.y, viewportSize.y - 20.f);

		UI::SetNextWindowSize(modalSize, ImGuiCond_Always);

		if (UI::BeginPopupModal("Project Manager", nullptr,
		        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			UI::PushFont("WorkSans", UI::FontMode::Regular, 24.f);
			TextCentered("Projects");
			UI::PopFont();

			UI::Spacing();
			UI::Spacing();

			static p::String folder;

			ImGui::BeginTable("table", 2);
			ImGui::TableNextColumn();
			{
				UI::PushFont("WorkSans", UI::FontMode::Regular, 18.f);
				UI::Text("Open");
				UI::PopFont();
				UI::Separator();
				UI::Spacing();

				UI::SetItemDefaultFocus();
				{
					UI::Text("Recent projects:");
					static const char* recentProjects[]{"Project.rift"};
					static int selectedN = 0;
					UI::SetNextItemWidth(-FLT_MIN);

					for (int n = 0; n < IM_ARRAYSIZE(recentProjects); ++n)
					{
						const bool isSelected = (selectedN == n);
						UI::BulletText(recentProjects[n]);
						UI::SameLine(ImGui::GetContentRegionAvail().x - 30.f);
						if (UI::SmallButton("open")) {}
					}
				}
				UI::Dummy({10.f, 40.f});
			}
			ImGui::TableNextColumn();
			{
				UI::PushFont("WorkSans", UI::FontMode::Regular, 18.f);
				UI::Text("Create");
				UI::PopFont();
				UI::Separator();
				UI::Spacing();

				UI::PushItemWidth(-32.f);
				UI::InputTextWithHint("##path", "project path...", folder);
				UI::PopItemWidth();
				UI::SameLine();
				if (UI::Button("...", p::v2{24.f, 0.f}))
				{
					p::Path selectedFolder =
					    p::files::SelectFolderDialog("Select project folder", p::GetCurrentPath());
					folder = p::ToString(selectedFolder);
				}
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (UI::Button("Open", p::v2{-FLT_MIN, 0.0f}))
			{
				p::String folder =
				    p::files::SelectFolderDialog("Select project folder", p::GetCurrentPath());
				if (Editor::Get().OpenProject(folder))
				{
					UI::CloseCurrentPopup();
				}
				else
				{
					UI::AddNotification({UI::ToastType::Error, 1.f,
					    p::Strings::Format("Failed to open project at '{}'", p::ToString(folder))});
				}
			}
			ImGui::TableNextColumn();
			if (UI::Button("Create", p::v2{-FLT_MIN, 0.0f}))
			{
				if (Editor::Get().CreateProject(folder))
				{
					folder = "";
					UI::CloseCurrentPopup();
				}
				else
				{
					UI::AddNotification({UI::ToastType::Error, 1.f,
					    p::Strings::Format("Failed to create project at '{}'", folder)});
				}
			}
			ImGui::EndTable();

			UI::EndPopup();
		}
	}

	void OpenProjectManager()
	{
		UI::OpenPopup("Project Manager");
	}
}    // namespace rift::Editor
