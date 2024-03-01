// Copyright 2015-2023 Piperift - All rights reserved

#include "Systems/EditorSystem.h"

#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeUtils.h"
#include "Components/CModuleEditor.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "Editor.h"
#include "imgui.h"
#include "Pipe/Core/String.h"
#include "Pipe/Files/Files.h"
#include "Statics/SEditor.h"
#include "Utils/DetailsPanel.h"
#include "Utils/ElementsPanel.h"
#include "Utils/FunctionGraph.h"
#include "Utils/ModuleUtils.h"
#include "Utils/ProjectManager.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Tags/CDirty.h>
#include <Compiler/Compiler.h>
#include <IconsFontAwesome5.h>
#include <MIRBackendModule.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>
#include <PipeArrays.h>
#include <PipeECS.h>
#include <PipeFiles.h>
#include <Rift.h>
#include <UI/Inspection.h>
#include <UI/Notify.h>
#include <UI/UI.h>


namespace rift::editor::EditorSystem
{
	void OnProjectEditorOpen(ast::Tree& ast)
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

	void OnTypeEditorOpen(ast::Tree& ast, ast::Id typeId)
	{
		auto& typeEditor = ast.Get<CTypeEditor>(typeId);
		typeEditor.layout.OnBuild([](auto& builder) {
			// ==================================== //
			//                           |          //
			//                           | Elements //
			//            Graph          |          //
			//                           |----------//
			//                           | Details  //
			//                           |          //
			// ==================================== //
			Tag rightNode{"rightNode"};
			builder.Split(
			    builder.GetRootNode(), ImGuiDir_Left, 0.5f, CTypeEditor::centralNode, rightNode);

			builder.Split(rightNode, ImGuiDir_Up, 0.4f, CTypeEditor::rightTopNode,
			    CTypeEditor::rightBottomNode);

			builder.GetNodeLocalFlags(CTypeEditor::centralNode) |=
			    i32(ImGuiDockNodeFlags_AutoHideTabBar);
		});

		// TODO: Reseting until we are able to know if the layout was saved before. Reset if it
		// didn't exist
		typeEditor.layout.Reset();
	}

	void Init(ast::Tree& ast)
	{
		OnProjectEditorOpen(ast);
		ast.OnAdd<CTypeEditor>().Bind([](auto& ast, auto ids) {
			for (ast::Id id : ids)
			{
				OnTypeEditorOpen(static_cast<ast::Tree&>(ast), id);
			}
		});
	}

	// Root Editor
	void CreateRootDockspace(SEditor& editor);
	void CreateTypeDockspace(CTypeEditor& editor, const char* id);
	void CreateModuleDockspace(CModuleEditor& editor, const char* id);
	void DrawMenuBar(ast::Tree& ast);

	// Project Editor
	void DrawProject(ast::Tree& ast);
	void DrawProjectMenuBar(ast::Tree& ast, SEditor& editorData);

	// Module Editors
	void DrawModuleEditors(ast::Tree& ast, SEditor& editor);

	// Type Editors
	void DrawTypeMenuBar(ast::Tree& ast, ast::Id typeId);
	void DrawTypes(ast::Tree& ast, SEditor& editor);


	void Draw(ast::Tree& ast)
	{
		if (ast::HasProject(ast))
		{
			DrawProject(ast);
		}
		else
		{
			OpenProjectManager();
		}

		DrawMenuBar(ast);
		DrawProjectManager(ast);

#if P_DEBUG
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
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		editor.dockspaceID = UI::GetID(id);
		editor.layout.Tick(editor.dockspaceID);
		UI::DockSpace(editor.dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	}

	void DrawMenuBar(ast::Tree& ast)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("Views"))
			{
#if P_DEBUG
				UI::MenuItem("Style", nullptr, &Editor::Get().showStyle);
				UI::MenuItem("Metrics", nullptr, &Editor::Get().showMetrics);
				UI::MenuItem("Demo", nullptr, &Editor::Get().showDemo);
#endif
				UI::EndMenu();
			}

			UI::EndMainMenuBar();
		}
	}

	void DrawProject(ast::Tree& ast)
	{
		if (!Ensure(ast.HasStatic<SEditor>()))
		{
			return;
		}
		auto& editor = ast.GetStatic<SEditor>();

		const auto& path = ast::GetProjectPath(ast);
		UI::PushID(p::GetHash(path));

		DrawProjectMenuBar(ast, editor);

		if (editor.skipFrameAfterMenu)    // We could have closed the project
		{
			editor.skipFrameAfterMenu = false;
			UI::PopID();
			return;
		}

		CreateRootDockspace(editor);

		DrawModuleEditors(ast, editor);
		DrawTypes(ast, editor);

		editor.reflectionDebugger.Draw();
		editor.ASTDebugger.Draw(ast);
		editor.memoryDebugger.Draw();
		editor.fileExplorer.Draw(ast);
		editor.graphPlayground.Draw(ast, editor.layout);

		UI::PopID();
	}

	void DrawProjectMenuBar(ast::Tree& ast, SEditor& editorData)
	{
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("File"))
			{
				if (UI::MenuItem("Open Project"))
				{
					const p::String folder = p::SelectFolderDialog(
					    "Select project folder", p::PlatformPaths::GetCurrentPath());
					if (Editor::Get().OpenProject(folder))
					{
						editorData.skipFrameAfterMenu = true;
					}
				}
				if (UI::MenuItem("Close current"))
				{
					Editor::Get().CloseProject();
					editorData.skipFrameAfterMenu = true;
				}
				UI::Separator();
				if (UI::MenuItem("Open File")) {}
				if (UI::MenuItem(ICON_FA_SAVE " Save All", "CTRL+SHFT+S"))
				{
					TArray<TPair<String, String>> fileDatas;    // Path to file data

					auto dirtyTypeIds =
					    FindAllIdsWith<ast::CDeclType, CTypeEditor, ast::CFileRef, ast::CFileDirty>(
					        ast);
					for (ast::Id typeId : dirtyTypeIds)
					{
						auto& file     = ast.Get<ast::CFileRef>(typeId);
						auto& fileData = fileDatas.AddRef({file.path, ""});
						ast::SerializeType(ast, typeId, fileData.second);
					}

					auto dirtyModuleIds =
					    FindAllIdsWith<ast::CModule, CModuleEditor, ast::CFileRef, ast::CFileDirty>(
					        ast);
					for (ast::Id moduleId : dirtyModuleIds)
					{
						auto& file     = ast.Get<ast::CFileRef>(moduleId);
						auto& fileData = fileDatas.AddRef({file.path, ""});
						ast::SerializeModule(ast, moduleId, fileData.second);
					}

					for (auto& fileData : fileDatas)
					{
						SaveStringFile(fileData.first, fileData.second);
					}

					ast.Remove<ast::CFileDirty>(dirtyTypeIds);
					ast.Remove<ast::CFileDirty>(dirtyModuleIds);

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
					ast::Tree compileAST{ast};    // Intentional copy
					CompilerConfig config;
					Build<MIRBackend>(compileAST, config);
				}
				if (UI::MenuItem("Build all"))
				{
					ast::Tree compileAST{ast};    // Intentional copy
					CompilerConfig config;
					Build<MIRBackend>(compileAST, config);
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
					UI::PushStyleCompact();
					if (UI::SliderFloat("Grid Size", &gridSize, 8.f, 32.f, "%.f"))
					{
						Graph::settings.SetGridSize(gridSize);
					}
					UI::PopStyleCompact();
					UI::EndMenu();
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Views"))
			{
				if (UI::BeginMenu("Debug"))
				{
					UI::MenuItem("Reflection", nullptr, &editorData.reflectionDebugger.open);
					UI::MenuItem("Abstract Syntax Tree", nullptr, &editorData.ASTDebugger.open);
					UI::MenuItem("Memory", nullptr, &editorData.memoryDebugger.open);
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

					for (ast::Id typeId : FindAllIdsWith<CTypeEditor>(ast))
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

	void DrawModuleMenuBar(ast::Tree& ast, ast::Id moduleId)
	{
		if (UI::BeginMenuBar())
		{
			if (UI::MenuItem(ICON_FA_SAVE, "CTRL+S"))
			{
				auto& file = ast.Get<ast::CFileRef>(moduleId);
				String data;
				ast::SerializeModule(ast, moduleId, data);

				SaveStringFile(StringView(file.path), data);
				ast.Remove<ast::CFileDirty>(moduleId);

				UI::AddNotification({UI::ToastType::Success, 1.f,
				    Strings::Format("Saved file {}", p::GetFilename(file.path))});
			}
			UI::EndMenuBar();
		}
	}

	void DrawModuleEditors(ast::Tree& ast, SEditor& editor)
	{
		TAccess<TWrite<CModuleEditor>, TWrite<ast::CNamespace>, TWrite<ast::CModule>, ast::CFileRef>
		    moduleEditors{ast};
		for (ast::Id moduleId :
		    FindAllIdsWith<ast::CModule, CModuleEditor, ast::CFileRef>(moduleEditors))
		{
			auto& moduleEditor = moduleEditors.Get<CModuleEditor>(moduleId);
			const auto& file   = moduleEditors.Get<const ast::CFileRef>(moduleId);

			bool isOpen               = true;
			const String path         = p::ToString(file.path);
			const StringView filename = p::GetFilename(path);
			const StringView dirty    = ast.Has<ast::CFileDirty>(moduleId) ? " *" : "";
			const String windowName =
			    Strings::Format(ICON_FA_TH_LARGE " {}{}###{}", filename, dirty, moduleId);

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
					auto& module = moduleEditors.Get<ast::CModule>(moduleId);
					UI::InspectStruct(&module);

					if (UI::BeginCategory("Bindings", true))
					{
						for (const auto& binding : ast::GetModuleBindings())
						{
							auto* pool = ast.GetPool(binding.tagType->GetId());
							if (void* data = pool ? pool->TryGetVoid(moduleId) : nullptr)
							{
								if (UI::BeginCategory(binding.displayName, true))
								{
									UI::InspectChildrenProperties({data, binding.tagType});
									UI::EndCategory();
								}
							}
							else
							{
								UI::PushStyleCompact();
								const p::String addText =
								    p::Strings::Format(ICON_FA_PLUS " {}", binding.displayName);
								UI::TableNextRow();
								UI::TableSetColumnIndex(1);
								if (UI::Button(addText.c_str(), ImVec2(-FLT_MIN, 0.0f)))
								{
									ScopedChange(ast, moduleId);
									ast::AddBindingToModule(ast, moduleId, binding.id);
								}
								UI::PopStyleCompact();
							}
						}
						UI::EndCategory();
					}
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
				CloseModuleEditor(ast, moduleId);
			}
		}
	}

	void DrawTypeMenuBar(ast::Tree& ast, ast::Id typeId)
	{
		auto& typeEditor = ast.Get<CTypeEditor>(typeId);
		if (UI::BeginMenuBar())
		{
			if (UI::MenuItem(ICON_FA_SAVE, "CTRL+S"))
			{
				auto& file = ast.Get<ast::CFileRef>(typeId);
				String data;
				ast::SerializeType(ast, typeId, data);

				SaveStringFile(StringView(file.path), data);
				ast.Remove<ast::CFileDirty>(typeId);

				UI::AddNotification({UI::ToastType::Success, 1.f,
				    Strings::Format("Saved file {}", p::GetFilename(file.path))});
			}

			if (UI::BeginMenu("View"))
			{
				if (ast::HasFunctions(ast, typeId))
				{
					UI::MenuItem("Graph", nullptr, &typeEditor.showGraph);
				}
				UI::MenuItem("Elements", nullptr, &typeEditor.showElements);
				UI::MenuItem("Details", nullptr, &typeEditor.showDetails);
				UI::EndMenu();
			}
			UI::EndMenuBar();
		}
	}

	void DrawTypes(ast::Tree& ast, SEditor& editor)
	{
		TAccess<TWrite<CTypeEditor>, ast::CDeclType, ast::CFileRef, ast::CDeclClass,
		    ast::CDeclStruct, ast::CDeclStatic>
		    access{ast};
		for (ast::Id typeId : FindAllIdsWith<ast::CDeclType, CTypeEditor, ast::CFileRef>(access))
		{
			auto& typeEditor = access.Get<CTypeEditor>(typeId);
			const auto& file = access.Get<const ast::CFileRef>(typeId);


			StringView icon;
			if (ast::IsStructType(access, typeId))
				icon = ICON_FA_FILE_ALT;
			else if (ast::IsClassType(access, typeId))
				icon = ICON_FA_FILE_INVOICE;
			else if (ast::IsStaticType(access, typeId))
				icon = ICON_FA_FILE_WORD;

			bool isOpen               = true;
			const String path         = p::ToString(file.path);
			const StringView filename = p::GetFilename(path);
			const StringView dirty    = ast.Has<ast::CFileDirty>(typeId) ? " *" : "";
			const String windowName =
			    Strings::Format("{} {}{}###{}", icon, filename, dirty, typeId);

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

				if (ast::HasFunctionBodies(ast, typeId))
				{
					Graph::DrawTypeGraph(ast, typeId, typeEditor);
				}

				if (ast::HasVariables(ast, typeId) || ast::HasFunctions(ast, typeId))
				{
					typeEditor.layout.BindNextWindowToNode(
					    CTypeEditor::rightBottomNode, ImGuiCond_Appearing);
					DrawDetailsPanel(ast, typeId);

					typeEditor.layout.BindNextWindowToNode(
					    CTypeEditor::rightTopNode, ImGuiCond_Appearing);
					DrawElementsPanel(ast, typeId);
				}
			}
			else
			{
				UI::PopStyleVar(3);
			}
			UI::End();

			if (!isOpen)
			{
				CloseType(ast, typeId);
			}
		}
	}
}    // namespace rift::editor::EditorSystem
