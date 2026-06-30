// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Editor.h"
#include "Statics/EditorSettings.h"
#include "Utils/ElementsPanel.h"

#include <AST/Utils/Settings.h>
#include <Pipe/Files/PlatformPaths.h>
#include <PipeFiles.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace rift::editor
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

		UI::PushStyleVar(ImGuiStyleVar_WindowPadding, p::v2{24.f, 12.f});
		if (UI::BeginPopupModal("Project Manager", nullptr,
		        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			UI::PushFont("WorkSans", UI::FontMode::Regular, 24.f);
			TextCentered("Projects");
			UI::PopFont();

			UI::Spacing();
			UI::Spacing();

			static p::String folder;

			ImGui::BeginTable("table", 3);
			ImGui::TableSetupColumn("##create", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##gap", ImGuiTableColumnFlags_WidthFixed, 8.f);
			ImGui::TableSetupColumn("##recent", ImGuiTableColumnFlags_WidthStretch);
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
					folder = p::SelectFolderDialog(
					    "Select project folder", p::PlatformPaths::GetCurrentPath());
				}
			}
			ImGui::TableNextColumn();    // Gap
			ImGui::TableNextColumn();
			{
				UI::PushFont("WorkSans", UI::FontMode::Regular, 18.f);
				UI::Text("Recent");
				UI::PopFont();
				UI::Separator();
				UI::Spacing();

				UI::SetItemDefaultFocus();
				{
					auto& editorSettings = rift::GetUserSettings<EditorSettings>();
					UI::SetNextItemWidth(-FLT_MIN);

					for (int n = 0; n < editorSettings.recentProjects.Size(); ++n)
					{
						p::StringView path   = editorSettings.recentProjects[n];
						p::StringView name   = p::GetFilename(path);
						p::StringView parent = p::GetParentPath(path);

						if (UI::TextLink(name))
						{
							if (Editor::Get().OpenProject(path))
							{
								UI::CloseCurrentPopup();
							}
							else
							{
								UI::AddNotification({UI::ToastType::Error, 1.f,
								    p::Strings::Format("Failed to open project at '{}'", path)});
							}
						}
						UI::SameLine();
						UI::TextDisabled(parent);
					}
				}
				UI::Dummy({10.f, 40.f});
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (UI::Button("Open", p::v2{-FLT_MIN, 0.0f}))
			{
				p::String folder = p::SelectFolderDialog(
				    "Select project folder", p::PlatformPaths::GetCurrentPath());
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
			{}
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
		UI::PopStyleVar();
	}

	void OpenProjectManager()
	{
		UI::OpenPopup("Project Manager");
	}
}    // namespace rift::editor
