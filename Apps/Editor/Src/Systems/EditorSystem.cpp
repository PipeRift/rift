// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"
#include "EditorData.h"
#include "Files/FileDialog.h"
#include "Systems/EditorSystem.h"

#include <RiftContext.h>


namespace Rift::EditorSystem
{
	void DrawMenuBar(EditorData& editorData);
	void DrawProjectPickerPopup(AST::Tree& ast);


	void Draw(AST::Tree& ast, EditorData& editorData)
	{
		ZoneScopedN("EditorSystem::Draw");
		if (Modules::HasProject(ast))
		{
			editorData.projectEditor->Draw();
		}
		else
		{
			UI::OpenPopup("Project Picker");
		}

		DrawMenuBar(editorData);
		DrawProjectPickerPopup(ast);

		editorData.memoryDebugger.Draw();

#if BUILD_DEBUG
		if (editorData.showDemo)
		{
			UI::ShowDemoWindow(&editorData.showDemo);
		}
#endif
	}

	void DrawMenuBar(EditorData& editorData)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("Views"))
			{
				UI::MenuItem("Memory", nullptr, &editorData.memoryDebugger.open);
				UI::EndMenu();
			}

#if BUILD_DEBUG
			UI::MenuItem("Demo", nullptr, &editorData.showDemo);
#endif
			UI::EndMainMenuBar();
		}
	}

	void DrawProjectPickerPopup(AST::Tree& ast)
	{
		// Center modal when appearing
		UI::SetNextWindowPos(
		    UI::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (UI::BeginPopupModal("Project Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (UI::Button("Open Project..."))
			{
				Path folder = Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());

				if (Modules::OpenProject(ast, folder))
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
}    // namespace Rift::EditorSystem
