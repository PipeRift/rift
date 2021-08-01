// Copyright 2015-2020 Piperift - All rights reserved

#include "Systems/EditorSystem.h"

#include "Components/CTypeEditor.h"
#include "Editor.h"
#include "Files/FileDialog.h"
#include "NodeGraph/NodeGraphPanel.h"
#include "Uniques/CEditorUnique.h"
#include "Utils/FunctionGraph.h"
#include "Utils/Properties.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CType.h>
#include <AST/Utils/DeclarationUtils.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <RiftContext.h>


namespace Rift::EditorSystem
{
	// Root Editor
	void CreateDockspace(CEditorUnique& editor);
	void CreateDockspace(CTypeEditor& editor, const char* id);
	void DrawMenuBar(AST::Tree& ast);
	void DrawProjectPickerPopup(AST::Tree& ast);

	// Project Editor
	void DrawProject(AST::Tree& ast);
	void DrawProjectMenuBar(AST::Tree& ast, CEditorUnique& editorData);

	// Type Editor
	void DrawTypes(AST::Tree& ast, CEditorUnique& editor);


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

	void CreateDockspace(CTypeEditor& editor, const char* id)
	{
		ZoneScoped;
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		editor.dockspaceID = UI::GetID(id);
		UI::DockSpace(editor.dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
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


	void DrawProject(AST::Tree& ast)
	{
		ZoneScopedN("EditorSystem::DrawProject");

		if (!Ensure(ast.HasUnique<CEditorUnique>()))
		{
			return;
		}
		CEditorUnique& editor = ast.GetUnique<CEditorUnique>();

		const auto& path = Modules::GetProjectPath(ast);
		UI::PushID(Hash<Path>()(path));

		DrawProjectMenuBar(ast, editor);

		if (editor.skipFrameAfterMenu)    // We could have closed the project
		{
			editor.skipFrameAfterMenu = false;
			UI::PopID();
			return;
		}

		CreateDockspace(editor);
		editor.layout.Tick(editor.dockspaceID);

		DrawTypes(ast, editor);

		editor.astDebugger.Draw(ast);
		editor.fileExplorer.Draw(ast);

		UI::PopID();
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
						editorData.skipFrameAfterMenu = true;
					}
				}
				if (UI::MenuItem("Close current"))
				{
					Modules::CloseProject(ast);
					editorData.skipFrameAfterMenu = true;
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

					auto openTypes = ast.MakeView<CTypeEditor>();
					for (AST::Id typeId : openTypes)
					{
						auto& editor = openTypes.Get<CTypeEditor>(typeId);
						editor.layout.Reset();
					}
				}
				UI::EndMenu();
			}
			UI::EndMainMenuBar();
		}
	}

	void DrawTypes(AST::Tree& ast, CEditorUnique& editor)
	{
		auto typeEditors = ast.MakeView<CType, CTypeEditor>();
		for (AST::Id typeId : typeEditors)
		{
			ZoneScopedN("Draw Type");
			UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
			UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			auto& type       = typeEditors.Get<CType>(typeId);
			auto& typeEditor = typeEditors.Get<CTypeEditor>(typeId);

			bool isOpen               = true;
			const String path         = Paths::ToString(type.path);
			const StringView filename = Paths::GetFilename(path);
			const String windowName   = Strings::Format(TX("{}###{}"), filename, path);

			if (typeEditor.pendingFocus)
			{
				UI::SetWindowFocus(windowName.c_str());
				typeEditor.pendingFocus = false;
			}

			editor.layout.BindNextWindowToNode(CEditorUnique::centralNode);
			if (UI::Begin(windowName.c_str(), &isOpen))
			{
				UI::PopStyleVar(3);

				CreateDockspace(typeEditor, windowName.c_str());
				typeEditor.layout.Tick(typeEditor.dockspaceID);

				if (Declarations::IsStruct(ast, typeId))
				{
					DrawFunctionGraph(ast, AST::NoId, typeEditor.layout);
				}
				DrawProperties(ast, typeId, typeEditor.layout);
			}
			else
			{
				UI::PopStyleVar(3);
			}
			UI::End();

			if (!isOpen)
			{
				Types::CloseType(ast, typeId);
			}
		}
	}
}    // namespace Rift::EditorSystem
