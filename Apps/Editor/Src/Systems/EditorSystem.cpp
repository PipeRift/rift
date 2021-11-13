// Copyright 2015-2020 Piperift - All rights reserved

#include "Systems/EditorSystem.h"

#include "Components/CTypeEditor.h"
#include "Editor.h"
#include "Files/FileDialog.h"
#include "Uniques/CEditorUnique.h"
#include "Utils/FunctionGraph.h"
#include "Utils/Properties.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CType.h>
#include <AST/Utils/DeclarationUtils.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <Containers/Array.h>
#include <RiftContext.h>


namespace Rift::EditorSystem
{
	void OnProjectEditorOpen(AST::Tree& ast)
	{
		auto& editor = ast.GetUnique<CEditorUnique>();
		editor.layout.OnBuild([](auto& builder) {
			// ==================================== //
			//          |                           //
			//          |                           //
			//   Left   |          Central          //
			//(Explorer)|          (Types)          //
			//          |                           //
			//          |                           //
			// ==================================== //
			builder.Split(builder.GetRootNode(), ImGuiDir_Left, 0.2f, CEditorUnique::leftNode,
			    CEditorUnique::centralNode);

			builder.GetNodeLocalFlags(CEditorUnique::leftNode) |= ImGuiDockNodeFlags_AutoHideTabBar;
			builder.GetNodeLocalFlags(CEditorUnique::centralNode) |= ImGuiDockNodeFlags_CentralNode;
		});

		// TODO: Reseting until we are able to know if the layout was saved before. Reset if it
		// didn't exist
		editor.layout.Reset();
	}

	void OnTypeEditorOpen(AST::Tree::Registry& registry, AST::Id typeId)
	{
		auto& typeEditor = registry.get<CTypeEditor>(typeId);
		typeEditor.layout.OnBuild([](auto& builder) {
			// =================================== //
			//                           |         //
			//                           |         //
			//          Central          |  Right  //
			//          (Graph)          |(Details)//
			//                           |         //
			//                           |         //
			// =================================== //
			builder.Split(builder.GetRootNode(), ImGuiDir_Right, 0.2f, CTypeEditor::rightNode,
			    CTypeEditor::centralNode);

			builder.GetNodeLocalFlags(CTypeEditor::rightNode) |= ImGuiDockNodeFlags_AutoHideTabBar;
			builder.GetNodeLocalFlags(CTypeEditor::centralNode) |=
			    ImGuiDockNodeFlags_CentralNode | ImGuiDockNodeFlags_AutoHideTabBar;
		});

		// TODO: Reseting until we are able to know if the layout was saved before. Reset if it
		// didn't exist
		typeEditor.layout.Reset();
	}

	void Init(AST::Tree& ast)
	{
		OnProjectEditorOpen(ast);
		ast.OnConstruct<CTypeEditor>().connect<&OnTypeEditorOpen>();
	}

	// Root Editor
	void CreateRootDockspace(CEditorUnique& editor);
	void CreateTypeDockspace(CTypeEditor& editor, const char* id);
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
		if (bool& showDemo = Editor::Get().showDemo)
		{
			UI::ShowDemoWindow(&showDemo);
		}

		if (bool& showMetrics = Editor::Get().showMetrics)
		{
			ImGui::ShowMetricsWindow(&showMetrics);
		}

		if (bool& showStyle = Editor::Get().showStyle)
		{
			ImGui::Begin("Editor Style", &showStyle);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
#endif
	}

	void CreateRootDockspace(CEditorUnique& editor)
	{
		ZoneScoped;
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		const auto& viewport = UI::GetMainViewport();

		UI::SetNextWindowPos(viewport->WorkPos);
		UI::SetNextWindowSize(viewport->WorkSize);
		UI::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags hostWindowFlags =
		    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
		    | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking
		    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
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

	void CreateTypeDockspace(CTypeEditor& editor, const char* id)
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
#if BUILD_DEBUG
				UI::MenuItem("Style", nullptr, &Editor::Get().showStyle);
				UI::MenuItem("Metrics", nullptr, &Editor::Get().showMetrics);
				UI::MenuItem("Demo", nullptr, &Editor::Get().showDemo);
#endif
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


	void DrawProject(AST::Tree& ast)
	{
		ZoneScopedN("EditorSystem::DrawProject");

		if (!Ensure(ast.HasUnique<CEditorUnique>()))
		{
			return;
		}
		auto& editor = ast.GetUnique<CEditorUnique>();

		const auto& path = Modules::GetProjectPath(ast);
		UI::PushID(Hash<Path>()(path));

		DrawProjectMenuBar(ast, editor);

		if (editor.skipFrameAfterMenu)    // We could have closed the project
		{
			editor.skipFrameAfterMenu = false;
			UI::PopID();
			return;
		}

		CreateRootDockspace(editor);
		editor.layout.Tick(editor.dockspaceID);

		DrawTypes(ast, editor);

		editor.astDebugger.Draw(ast);
		editor.fileExplorer.Draw(ast);
		editor.graphPlayground.Draw(ast, editor.layout);

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
				if (UI::MenuItem("Save All", "CTRL+SHFT+S"))
				{
					// TODO: Only save dirty types
					auto typeEditors = ast.MakeView<CType, CTypeEditor, CFileRef>();
					TArray<TPair<Path, String>> fileDatas;
					for (AST::Id typeId : typeEditors)
					{
						auto& file     = typeEditors.Get<CFileRef>(typeId);
						auto& fileData = fileDatas.AddRef({file.path, ""});
						Types::Serialize(ast, typeId, fileData.second);
					}

					TaskSystem::Get().GetPool(TaskPool::Workers).silent_async([fileDatas]() {
						for (auto& fileData : fileDatas)
						{
							Files::SaveStringFile(fileData.first, fileData.second);
						}
					});
				}
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
				UI::Separator();
				if (UI::BeginMenu("Settings"))
				{
					float gridSize = Graph::settings.GetGridSize();
					Style::PushStyleCompact();
					if (UI::SliderFloat("Grid Size", &gridSize, 8.f, 32.f, "%.f"))
					{
						Graph::settings.SetGridSize(gridSize);
					}
					Style::PopStyleCompact();
					UI::EndMenu();
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Views"))
			{
				if (UI::BeginMenu("Debug"))
				{
					UI::MenuItem("Syntax Tree", nullptr, &editorData.astDebugger.open);
					UI::MenuItem("Graph Playground", nullptr, &editorData.graphPlayground.open);
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
		auto typeEditors = ast.MakeView<CType, CTypeEditor, CFileRef>();
		for (AST::Id typeId : typeEditors)
		{
			ZoneScopedN("Draw Type");

			auto& typeEditor = typeEditors.Get<CTypeEditor>(typeId);
			auto& file       = typeEditors.Get<CFileRef>(typeId);

			bool isOpen               = true;
			const String path         = Paths::ToString(file.path);
			const StringView filename = Paths::GetFilename(path);
			const String windowName   = Strings::Format("{}##{}", filename, typeId);

			if (typeEditor.pendingFocus)
			{
				UI::SetWindowFocus(windowName.c_str());
				typeEditor.pendingFocus = false;
			}

			editor.layout.BindNextWindowToNode(CEditorUnique::centralNode);

			UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
			UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if (UI::Begin(windowName.c_str(), &isOpen))
			{
				UI::PopStyleVar(3);

				CreateTypeDockspace(typeEditor, windowName.c_str());
				typeEditor.layout.Tick(typeEditor.dockspaceID);

				if (Declarations::IsClass(ast, typeId)
				    || Declarations::IsFunctionLibrary(ast, typeId))
				{
					Graph::DrawTypeGraph(ast, typeId, typeEditor);
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
