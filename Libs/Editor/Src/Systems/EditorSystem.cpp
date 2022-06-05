// Copyright 2015-2022 Piperift - All rights reserved

#include "Systems/EditorSystem.h"

#include "AST/Utils/TypeUtils.h"
#include "Components/CModuleEditor.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Editor.h"
#include "Files/Paths.h"
#include "imgui_internal.h"
#include "Statics/SEditor.h"
#include "Utils/FunctionGraph.h"
#include "Utils/ModuleUtils.h"
#include "Utils/ProjectManager.h"
#include "Utils/Properties.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CType.h>
#include <AST/Components/Tags/CDirty.h>
#include <Compiler/Compiler.h>
#include <Core/Array.h>
#include <CppBackend.h>
#include <ECS/Filtering.h>
#include <Files/FileDialog.h>
#include <IconsFontAwesome5.h>
#include <LLVMBackend.h>
#include <RiftContext.h>
#include <UI/Inspection.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace rift::EditorSystem
{
	void OnProjectEditorOpen(AST::Tree& ast)
	{
		auto& editor = ast.GetStatic<SEditor>();
		editor.layout.OnBuild([](auto& builder) {
			// ==================================== //
			//          |                           //
			//          |                           //
			//   Left   |          Central          //
			//(Explorer)|          (Files)          //
			//          |                           //
			//          |                           //
			// ==================================== //
			builder.Split(builder.GetRootNode(), ImGuiDir_Left, 0.2f, SEditor::leftNode,
			    SEditor::centralNode);

			builder.GetNodeLocalFlags(SEditor::centralNode) |= ImGuiDockNodeFlags_CentralNode;
		});

		// TODO: Reseting until we are able to know if the layout was saved before. Reset if it
		// didn't exist
		editor.layout.Reset();
	}

	void OnTypeEditorOpen(AST::Tree& ast, AST::Id typeId)
	{
		auto& typeEditor = ast.Get<CTypeEditor>(typeId);
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
		ast.OnAdd<CTypeEditor>().Bind([](auto& ast, auto ids) {
			for (AST::Id id : ids)
			{
				OnTypeEditorOpen(static_cast<AST::Tree&>(ast), id);
			}
		});
	}

	// Root Editor
	void CreateRootDockspace(SEditor& editor);
	void CreateTypeDockspace(CTypeEditor& editor, const char* id);
	void CreateModuleDockspace(CModuleEditor& editor, const char* id);
	void DrawMenuBar(AST::Tree& ast);

	// Project Editor
	void DrawProject(AST::Tree& ast);
	void DrawProjectMenuBar(AST::Tree& ast, SEditor& editorData);

	// Module Editors
	void DrawModules(AST::Tree& ast, SEditor& editor);

	// Type Editors
	void DrawTypeMenuBar(AST::Tree& ast, AST::Id typeId);
	void DrawTypes(AST::Tree& ast, SEditor& editor);


	void Draw(AST::Tree& ast)
	{
		ZoneScoped;

		if (Modules::HasProject(ast))
		{
			DrawProject(ast);
		}
		else
		{
			OpenProjectManager();
		}

		DrawMenuBar(ast);
		DrawProjectManager(ast);

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

		UI::DrawNotifications();
	}

	void CreateRootDockspace(SEditor& editor)
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
		editor.layout.Tick(editor.dockspaceID);
		UI::DockSpace(editor.dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
		UI::End();
	}

	void CreateTypeDockspace(CTypeEditor& editor, const char* id)
	{
		ZoneScoped;
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		editor.dockspaceID = UI::GetID(id);
		editor.layout.Tick(editor.dockspaceID);
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

	void DrawProject(AST::Tree& ast)
	{
		ZoneScoped;

		if (!Ensure(ast.HasStatic<SEditor>()))
		{
			return;
		}
		auto& editor = ast.GetStatic<SEditor>();

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

		DrawModules(ast, editor);
		DrawTypes(ast, editor);

		editor.reflectionDebugger.Draw();
		editor.astDebugger.Draw(ast);
		editor.fileExplorer.Draw(ast);
		editor.graphPlayground.Draw(ast, editor.layout);

		UI::PopID();
	}

	void DrawProjectMenuBar(AST::Tree& ast, SEditor& editorData)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("File"))
			{
				if (UI::MenuItem("Open Project"))
				{
					const pipe::Path folder =
					    Files::SelectFolderDialog("Select project folder", pipe::GetCurrentPath());
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
				if (UI::MenuItem(ICON_FA_SAVE " Save All", "CTRL+SHFT+S"))
				{
					TArray<TPair<Path, String>> fileDatas;

					auto dirtyTypeIds = ECS::ListAll<CType, CTypeEditor, CFileRef, CFileDirty>(ast);
					for (AST::Id typeId : dirtyTypeIds)
					{
						auto& file     = ast.Get<CFileRef>(typeId);
						auto& fileData = fileDatas.AddRef({file.path, ""});
						Types::Serialize(ast, typeId, fileData.second);
					}

					auto dirtyModuleIds =
					    ECS::ListAll<CModule, CModuleEditor, CFileRef, CFileDirty>(ast);
					for (AST::Id moduleId : dirtyModuleIds)
					{
						auto& file     = ast.Get<CFileRef>(moduleId);
						auto& fileData = fileDatas.AddRef({file.path, ""});
						Modules::Serialize(ast, moduleId, fileData.second);
					}

					for (auto& fileData : fileDatas)
					{
						Files::SaveStringFile(fileData.first, fileData.second);
					}

					ast.Remove<CFileDirty>(dirtyTypeIds);
					ast.Remove<CFileDirty>(dirtyModuleIds);

					UI::AddNotification({UI::ToastType::Success, 1.f,
					    !fileDatas.IsEmpty() ? Strings::Format("Saved {} files", fileDatas.Size())
					                         : "Nothing to save"});
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Build"))
			{
				if (UI::MenuItem("Build current"))
				{
					AST::Tree compileAST{ast};    // Intentional copy
					Compiler::Config config;
					Compiler::Build<Compiler::LLVMBackend>(compileAST, config);
				}
				if (UI::MenuItem("Build all"))
				{
					AST::Tree compileAST{ast};    // Intentional copy
					Compiler::Config config;
					Compiler::Build<Compiler::LLVMBackend>(compileAST, config);
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
					UI::MenuItem("Reflection", nullptr, &editorData.reflectionDebugger.open);
					UI::MenuItem("Abstract Syntax Tree", nullptr, &editorData.astDebugger.open);
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

					for (AST::Id typeId : ECS::ListAll<CTypeEditor>(ast))
					{
						auto& editor = ast.Get<CTypeEditor>(typeId);
						editor.layout.Reset();
					}
				}
				UI::EndMenu();
			}
			UI::EndMainMenuBar();
		}
	}

	void DrawModuleMenuBar(AST::Tree& ast, AST::Id moduleId)
	{
		if (UI::BeginMenuBar())
		{
			if (UI::MenuItem(ICON_FA_SAVE, "CTRL+S"))
			{
				auto& file = ast.Get<CFileRef>(moduleId);
				TPair<Path, String> fileData{file.path, ""};
				Modules::Serialize(ast, moduleId, fileData.second);

				Files::SaveStringFile(fileData.first, fileData.second);
				ast.Remove<CFileDirty>(moduleId);

				UI::AddNotification({UI::ToastType::Success, 1.f,
				    Strings::Format("Saved file {}", pipe::GetFilename(file.path))});
			}
			UI::EndMenuBar();
		}
	}

	void DrawModules(AST::Tree& ast, SEditor& editor)
	{
		TAccess<TWrite<CModuleEditor>, TWrite<CModule>, TWrite<CIdentifier>, CFileRef>
		    moduleEditors{ast};
		for (AST::Id moduleId : ECS::ListAll<CModule, CModuleEditor, CFileRef>(moduleEditors))
		{
			ZoneScopedN("Draw Type");

			auto& moduleEditor = moduleEditors.Get<CModuleEditor>(moduleId);
			const auto& file   = moduleEditors.Get<const CFileRef>(moduleId);

			bool isOpen               = true;
			const String path         = pipe::ToString(file.path);
			const StringView filename = pipe::GetFilename(path);
			const StringView dirty    = ast.Has<CFileDirty>(moduleId) ? "*" : "";
			const String windowName   = Strings::Format("{}{}###{}", filename, dirty, moduleId);

			if (moduleEditor.pendingFocus)
			{
				UI::SetWindowFocus(windowName.c_str());
				moduleEditor.pendingFocus = false;
			}

			editor.layout.BindNextWindowToNode(SEditor::centralNode, ImGuiCond_Appearing);

			UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
			UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if (UI::Begin(windowName.c_str(), &isOpen, ImGuiWindowFlags_MenuBar))
			{
				UI::PopStyleVar(3);

				DrawModuleMenuBar(ast, moduleId);

				if (UI::BeginInspector("ModuleInspector"))
				{
					auto& ident = moduleEditors.GetOrAdd<CIdentifier>(moduleId);
					UI::InspectStruct(&ident);
					auto& module = moduleEditors.Get<CModule>(moduleId);
					UI::InspectStruct(&module);
					UI::EndInspector();
				}
			}
			else
			{
				UI::PopStyleVar(3);
			}
			UI::End();

			if (!isOpen)
			{
				Modules::CloseEditor(ast, moduleId);
			}
		}
	}

	void DrawTypeMenuBar(AST::Tree& ast, AST::Id typeId)
	{
		if (UI::BeginMenuBar())
		{
			if (UI::MenuItem(ICON_FA_SAVE, "CTRL+S"))
			{
				auto& file = ast.Get<CFileRef>(typeId);
				TPair<Path, String> fileData{file.path, ""};
				Types::Serialize(ast, typeId, fileData.second);

				UI::AddNotification({UI::ToastType::Success, 1.f,
				    Strings::Format("Saved file {}", pipe::GetFilename(file.path))});

				Files::SaveStringFile(fileData.first, fileData.second);
				ast.Remove<CFileDirty>(typeId);
			}
			if (UI::BeginMenu("View"))
			{
				if (Types::CanContainFunctions(ast, typeId) && UI::MenuItem("Graph")) {}
				if (UI::MenuItem("Properties")) {}
				UI::EndMenu();
			}
			UI::EndMenuBar();
		}
	}

	void DrawTypes(AST::Tree& ast, SEditor& editor)
	{
		ZoneScoped;

		TAccess<TWrite<CTypeEditor>, CType, CFileRef> typeEditors{ast};
		for (AST::Id typeId : ECS::ListAll<CType, CTypeEditor, CFileRef>(typeEditors))
		{
			ZoneScopedN("Draw Type");

			auto& typeEditor = typeEditors.Get<CTypeEditor>(typeId);
			const auto& file = typeEditors.Get<const CFileRef>(typeId);

			bool isOpen               = true;
			const String path         = pipe::ToString(file.path);
			const StringView filename = pipe::GetFilename(path);
			const StringView dirty    = ast.Has<CFileDirty>(typeId) ? "*" : "";
			const String windowName   = Strings::Format("{}{}###{}", filename, dirty, typeId);

			if (typeEditor.pendingFocus)
			{
				UI::SetWindowFocus(windowName.c_str());
				typeEditor.pendingFocus = false;
			}

			editor.layout.BindNextWindowToNode(SEditor::centralNode, ImGuiCond_Appearing);

			UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
			UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if (UI::Begin(windowName.c_str(), &isOpen, ImGuiWindowFlags_MenuBar))
			{
				UI::PopStyleVar(3);

				DrawTypeMenuBar(ast, typeId);

				CreateTypeDockspace(typeEditor, windowName.c_str());

				if (Types::CanContainFunctions(ast, typeId))
				{
					Graph::DrawTypeGraph(ast, typeId, typeEditor);
				}

				Name propertiesNode = CTypeEditor::rightNode;
				if (!Types::CanContainFunctions(ast, typeId))
				{
					propertiesNode = DockSpaceLayout::rootNodeId;
				}
				typeEditor.layout.BindNextWindowToNode(propertiesNode, ImGuiCond_Appearing);
				DrawProperties(ast, typeId);
			}
			else
			{
				UI::PopStyleVar(3);
			}
			UI::End();

			if (!isOpen)
			{
				Types::CloseEditor(ast, typeId);
			}
		}
	}
}    // namespace rift::EditorSystem
