// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Editor.h"
#include "Statics/EditorSettings.h"
#include "Utils/ElementsPanel.h"

#include <AST/Utils/Settings.h>
#include <IconsFontAwesome5.h>
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

					p::i32 idxToRemove           = -1;
					const float spacing          = ImGui::GetStyle().ItemSpacing.x;
					const float removeButtonSize = 18.f;
					for (int n = 0; n < editorSettings.recentProjects.Size(); ++n)
					{
						p::StringView path   = editorSettings.recentProjects[n];
						p::StringView name   = p::GetFilename(path);
						p::StringView parent = p::GetParentPath(path);

						// Use the full path as the ID so projects sharing a name but living in
						// different folders are treated as distinct buttons.
						UI::PushID(path);
						if (UI::TextLink(name))
						{
							if (Editor::Get().OpenProject(path))
							{
								UI::CloseCurrentPopup();
							}
							else
							{
								UI::AddNotification({UI::ToastType::Error, 1.f,
								    p::Format("Failed to open project at '{}'", path)});
							}
						}
						UI::SameLine();

						// Truncate the path to the space left before the remove button
						const float textWidth = p::Max(
						    ImGui::GetContentRegionAvail().x - removeButtonSize - spacing, 1.f);
						p::String display{parent};
						if (ImGui::CalcTextSize(display.data(), display.data() + display.size()).x
						    > textWidth)
						{
							const float ellipsisWidth = ImGui::CalcTextSize("...").x;
							while (!display.empty()
							       && ImGui::CalcTextSize(
							              display.data(), display.data() + display.size())
							                      .x
							                  + ellipsisWidth
							              > textWidth)
							{
								display.resize(display.size() - 1);
							}
							display += "...";
						}
						UI::TextDisabled(display);

						// Remove button, aligned to the right, no background (highlight on hover)
						const float widthAvailable =
						    ImGui::GetContentRegionAvail().x + UI::GetCurrentWindow()->DC.Indent.x;
						UI::SameLine(widthAvailable - removeButtonSize);

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.f, 0.f, 0.f, 0.f});
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{1.f, 1.f, 1.f, 0.12f});
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{1.f, 1.f, 1.f, 0.22f});
						UI::PushTextColor(UI::GetTextColor().Shade(0.2f));
						UI::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.5f, 0.5f});
						float backupPaddingY             = ImGui::GetStyle().FramePadding.y;
						ImGui::GetStyle().FramePadding.y = 0.f;
						if (ImGui::ButtonEx(ICON_FA_TIMES,
						        p::v2{removeButtonSize, removeButtonSize},
						        ImGuiButtonFlags_AlignTextBaseLine))
						{
							idxToRemove = n;
						}
						ImGui::GetStyle().FramePadding.y = backupPaddingY;
						UI::PopStyleVar();
						UI::PopTextColor();
						ImGui::PopStyleColor(3);
						UI::PopID();
					}

					if (idxToRemove >= 0)
					{
						editorSettings.recentProjects.RemoveAt(idxToRemove);
						SaveUserSettings<EditorSettings>();
					}
				}
				UI::Dummy({10.f, 40.f});
			}

			ImGui::TableNextRow();
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
					    p::Format("Failed to create project at '{}'", folder)});
				}
			}
			ImGui::TableNextColumn();
			{}
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
					    p::Format("Failed to open project at '{}'", folder)});
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
