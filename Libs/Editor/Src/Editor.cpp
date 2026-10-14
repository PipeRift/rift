// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Editor.h"

#include "AST/Systems/FunctionsSystem.h"
#include "AST/Utils/Namespaces.h"
#include "AST/Utils/Settings.h"
#include "Statics/EditorSettings.h"
#include "Statics/SEditor.h"
#include "Systems/EditorSystem.h"
#include "Utils/FunctionGraph.h"

#include <AST/Statics/SModules.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Systems/LoadSystem.h>
#include <AST/Systems/TransactionSystem.h>
#include <AST/Systems/TypeSystem.h>
#include <AST/Utils/ModuleUtils.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Files.h>
#include <UI/Inspection.h>
#include <UI/Window.h>


namespace rift::editor
{
	void RegisterKeyValueInspections()
	{
		UI::RegisterCustomInspection<ast::Id>([](p::StringView label, void* data, p::TypeId type)
		{
			auto* id = static_cast<ast::Id*>(data);
			// UI::DrawKeyValue(label, data, GetType<IdTraits<ast::Id>::Entity>());
			UI::TableNextRow();
			UI::TableSetColumnIndex(0);
			UI::AlignTextToFramePadding();
			UI::Text(label);
			UI::TableSetColumnIndex(1);
			String asString = p::ToString(*id);
			if (UI::InputText("##value", asString))
			{
				*id = p::IdFromString(asString, nullptr);
			}
		});

		UI::RegisterCustomInspection<ast::Namespace>(
		    [](p::StringView label, void* data, p::TypeId type)
		{
			UI::TableNextRow();
			UI::TableSetColumnIndex(0);
			UI::AlignTextToFramePadding();
			UI::Text(label);
			UI::TableSetColumnIndex(1);
			auto* ns        = static_cast<ast::Namespace*>(data);
			String asString = ns->ToString();
			if (UI::InputText("##value", asString))
			{
				*ns = ast::Namespace{asString};
			}
		});
	}

	int Editor::Run(StringView projectPath)
	{
		fileWatcher.StartWatchingAsync();

		// Setup window
		p::Info("Initializing editor...");
		if (!rift::UI::Init())
		{
			p::Error("Failed to initialize editor");
			return 1;
		}
		Graph::Init();
		RegisterKeyValueInspections();
		p::Info("Editor is ready");

		// Don't write an imgui.ini in the working directory while no project is open.
		ImGui::GetIO().IniFilename = nullptr;

		// Open a project if a path has been provided
		OpenProject(projectPath, false);

		while (!UI::WantsToClose())
		{
			frameTime.PreTick();

			UI::PreFrame();
			UpdateConfig();

			Tick();
			UI::Render();

			frameTime.PostTick();
		}

		// Close the current project (if any)
		CloseProject();

		Graph::Shutdown();
		UI::Shutdown();
		return 0;
	}

	Editor::~Editor()
	{
		UI::Shutdown();
	}

	void Editor::Tick()
	{
		if (ast::HasProject(ast))
		{
			ast::FunctionsSystem::ClearAddedTags(ast);
			ast::TransactionSystem::ClearTags(ast);

			if (filesDirty)
			{
				ast::LoadSystem::Run(ast);
				ast::TypeSystem::SyncTypesByPath(ast);
				filesDirty = false;
			}
			ast::FunctionsSystem::ResolveCallFunctionIds(ast);
			ast::TypeSystem::ResolveExprTypeIds(ast);

			// Repair any invalid graph connectivity before drawing it
			ast::FunctionsSystem::HealGraph(ast);

			EditorSystem::Draw(ast);
			ast::TypeSystem::PropagateVariableTypes(ast);
			ast::FunctionsSystem::PropagateDirtyIntoCalls(ast);
			ast::FunctionsSystem::PushInvalidPinsBack(ast);
			ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
			ast::FunctionsSystem::SyncReturnPinsFromFunction(ast);
			ast::TypeSystem::PropagateExpressionTypes(ast);
		}
		else
		{
			EditorSystem::Draw(ast);
		}
	}

	void Editor::SetUIConfigFile(p::StringView path)
	{
		if (UI::GetWindow())
		{
			// Ensure the project's Saved folder exists so the layout can persist there
			// instead of falling back to an imgui.ini in the working directory.
			p::CreateFolder(p::GetParentPath(path), true);
			configFileChanged          = true;
			configFile                 = path;
			ImGui::GetIO().IniFilename = configFile.c_str();
		}
	}

	bool Editor::CreateProject(p::StringView path, bool closeFirst)
	{
		if (!closeFirst && ast::HasProject(ast))
		{
			return false;
		}

		if (ast::CreateProject(ast, path) && ast::OpenProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			SetUIConfigFile(p::JoinPaths(ast::GetProjectPath(ast), "Saved/UI.ini"));
			filesDirty = true;
			return true;
		}
		return false;
	}

	bool Editor::OpenProject(p::StringView path, bool closeFirst)
	{
		if (!closeFirst && ast::HasProject(ast))
		{
			return false;
		}

		if (ast::OpenProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			auto projectPath = ast::GetProjectPath(ast);
			SetUIConfigFile(p::JoinPaths(projectPath, "Saved/UI.ini"));

			// Start watching the project folder for file changes
			ast.Add(GetProjectId(ast), fileWatcher.ListenPath(projectPath, true,
			                               [](FileWatchId id, StringView path, StringView filename,
			                                   FileWatchAction action, StringView oldFilename)
			{
				Editor::Get().filesDirty = true;
			}));


			auto& editorSettings = GetUserSettings<EditorSettings>();
			p::String projectPathStr{projectPath};
			p::SetWeaklyCanonical(projectPathStr);    // Prevent / \\ mismatch duplicates
			editorSettings.recentProjects.AddUnique(p::Move(projectPathStr));
			SaveUserSettings<EditorSettings>();
			filesDirty = true;
			return true;
		}
		return false;
	}

	void Editor::CloseProject()
	{
		Id id = GetProjectId(ast);
		if (ast.IsValid(id) && ast.Has<p::FileWatchId>(id))
		{
			fileWatcher.StopListening(ast.Get<p::FileWatchId>(id));
		}
		ast::CloseProject(ast);

		// Stop writing a project layout file and never fall back to imgui.ini
		configFile.clear();
		configFileChanged          = false;
		ImGui::GetIO().IniFilename = nullptr;
	}

	void Editor::Close()
	{
		rift::UI::Close();
	}

	void Editor::UpdateConfig()
	{
		if (configFileChanged)
		{
			ImGui::GetIO().IniFilename = configFile.c_str();
			if (configFile.empty())
			{
				return;
			}

			if (ExistsAsFile(configFile))
			{
				// FIX: Delay this until new frame (essentially, not while already drawing)
				ImGui::LoadIniSettingsFromDisk(configFile.c_str());
			}
			else
			{
				ImGui::SaveIniSettingsToDisk(configFile.c_str());
			}
			configFileChanged = false;
		}
	}
}    // namespace rift::editor
