// Copyright 2015-2020 Piperift - All rights reserved

#include "Editor.h"
#include "Files/FileDialog.h"
#include "Systems/EditorSystem.h"
#include "Uniques/CEditorUnique.h"

#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <RiftContext.h>


namespace Rift::EditorSystem
{
	void CreateDockspace(CEditorUnique& editor);

	void DrawProject(AST::Tree& ast);
	void DrawMenuBar(AST::Tree& ast);
	void DrawProjectMenuBar(AST::Tree& ast, CEditorUnique& editorData);
	void DrawProjectPickerPopup(AST::Tree& ast);


	void Draw(AST::Tree& ast)
	{
		ZoneScopedN("EditorSystem::Draw");


		if (Modules::HasProject(ast))
		{
			DrawProject(ast);
		}
		else
		{
			UI::OpenPopup("Project Picker");
		}

		DrawMenuBar(ast);
		DrawProjectPickerPopup(ast);

#if BUILD_DEBUG
		bool& showDemo = Editor::Get().showDemo;
		if (showDemo)
		{
			UI::ShowDemoWindow(&showDemo);
		}
#endif
	}

	void CreateDockspace(CEditorUnique& editor)
	{
		ZoneScoped;
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		const auto& viewport = UI::GetMainViewport();

		UI::SetNextWindowPos(viewport->WorkPos);
		UI::SetNextWindowSize(viewport->WorkSize);
		UI::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags hostWindowFlags =
		    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
		    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		if (dockingFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			hostWindowFlags |= ImGuiWindowFlags_NoBackground;
		}

		char label[32];
		ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

		UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		UI::Begin(label, nullptr, hostWindowFlags);
		UI::PopStyleVar(3);

		editor.dockspaceID = UI::GetID("DockSpace");
		UI::DockSpace(editor.dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
		UI::End();
	}

	void DrawProject(AST::Tree& ast)
	{
		ZoneScopedN("EditorSystem::DrawProject");

		CEditorUnique* editor = ast.TryGetUnique<CEditorUnique>();
		if (!Ensure(editor))
		{
			return;
		}

		const auto& path = Modules::GetProjectPath(ast);
		UI::PushID(Hash<Path>()(path));

		DrawProjectMenuBar(ast, *editor);

		if (editor->bSkipFrameAfterMenu)    // We could have closed the project
		{
			editor->bSkipFrameAfterMenu = false;
			UI::PopID();
			return;
		}

		CreateDockspace(*editor);
		editor->layout.Tick(editor->dockspaceID);

		editor->astDebugger.Draw(ast);
		editor->fileExplorer.Draw(ast);

		UI::PopID();
	}

	void DrawMenuBar(AST::Tree& ast)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("Views"))
			{
				UI::EndMenu();
			}

#if BUILD_DEBUG
			bool& showDemo = Editor::Get().showDemo;
			UI::MenuItem("Demo", nullptr, &showDemo);
#endif
			UI::EndMainMenuBar();
		}
	}

	void DrawProjectMenuBar(AST::Tree& ast, CEditorUnique& editorData)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("File"))
			{
				if (UI::MenuItem("Open Project"))
				{
					const Path folder =
					    Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());
					if (Editor::Get().OpenProject(folder))
					{
						editorData.bSkipFrameAfterMenu = true;
					}
				}
				if (UI::MenuItem("Close current"))
				{
					Modules::CloseProject(ast);
					editorData.bSkipFrameAfterMenu = true;
				}
				UI::Separator();
				if (UI::MenuItem("Open File")) {}
				if (UI::MenuItem("Save File", "CTRL+S")) {}
				if (UI::MenuItem("Save All", "CTRL+SHFT+S")) {}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Build"))
			{
				if (UI::MenuItem("Build current"))
				{
					AST::Tree compileAST;
					compileAST.CopyFrom(ast);
					Rift::Compiler::Config config;
					Rift::Compiler::Build(compileAST, config, Rift::Compiler::EBackend::Cpp);
				}
				if (UI::MenuItem("Build all"))
				{
					AST::Tree compileAST;
					compileAST.CopyFrom(ast);
					Rift::Compiler::Config config;
					Rift::Compiler::Build(compileAST, config, Rift::Compiler::EBackend::Cpp);
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Edit"))
			{
				if (UI::MenuItem("Undo", "CTRL+Z")) {}
				if (UI::MenuItem("Redo", "CTRL+Y", false, false)) {}    // Disabled item
				UI::Separator();
				if (UI::MenuItem("Cut", "CTRL+X")) {}
				if (UI::MenuItem("Copy", "CTRL+C")) {}
				if (UI::MenuItem("Paste", "CTRL+V")) {}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Views"))
			{
				if (UI::BeginMenu("Debug"))
				{
					UI::MenuItem("Syntax Tree", nullptr, &editorData.astDebugger.open);
					UI::EndMenu();
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Layout"))
			{
				if (UI::MenuItem("Reset layout"))
				{
					editorData.layout.Reset();
					// for (const auto& editor : typeAssetEditors)
					//{
					//	if (editor)
					//	{
					//		editor->GetLayout().Reset();
					//	}
					//}
				}
				UI::EndMenu();
			}
			UI::EndMainMenuBar();
		}
	}

	void DrawProjectPickerPopup(AST::Tree& ast)
	{
		// Center modal when appearing
		UI::SetNextWindowPos(UI::GetMainViewport()->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});

		if (UI::BeginPopupModal("Project Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (UI::Button("Open Project..."))
			{
				Path folder = Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());
				if (Editor::Get().OpenProject(folder))
				{
					UI::CloseCurrentPopup();
				}
			}
			UI::SetItemDefaultFocus();
			UI::Separator();
			{
				UI::Text("Recent Projects");
				static const char* recentProjects[]{"One recent project"};
				static int selectedN = 0;
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
			UI::EndPopup();
		}
	}
}    // namespace Rift::EditorSystem
