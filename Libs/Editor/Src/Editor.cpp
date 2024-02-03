// Copyright 2015-2023 Piperift - All rights reserved

#include "Editor.h"

#include "AST/Systems/FunctionsSystem.h"
#include "AST/Utils/Namespaces.h"
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


namespace rift::Editor
{
	void RegisterKeyValueInspections()
	{
		UI::RegisterCustomInspection<ast::Id>([](StringView label, void* data, Type* type) {
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
				*id = p::IdFromString(asString);
			}
		});

		UI::RegisterCustomInspection<ast::Namespace>([](StringView label, void* data, Type* type) {
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
		fileWatcher.StartAsync();

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

			if (bFilesDirty)
			{
				ast::LoadSystem::Run(ast);
				bFilesDirty = false;
			}
			ast::FunctionsSystem::ResolveCallFunctionIds(ast);
			ast::TypeSystem::ResolveExprTypeIds(ast);

			EditorSystem::Draw(ast);
			ast::TypeSystem::PropagateVariableTypes(ast);
			ast::FunctionsSystem::PropagateDirtyIntoCalls(ast);
			ast::FunctionsSystem::PushInvalidPinsBack(ast);
			ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
			ast::TypeSystem::PropagateExpressionTypes(ast);
		}
		else
		{
			EditorSystem::Draw(ast);
		}
	}

	void Editor::SetUIConfigFile(Path path)
	{
		if (UI::GetWindow())
		{
			configFileChanged          = true;
			configFile                 = p::ToString(path);
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
			                               [](StringView path, StringView filename,
			                                   FileWatchAction action, StringView oldFilename) {
				Editor::Get().bFilesDirty = true;
			}));

			return true;
		}
		return false;
	}

	void Editor::CloseProject()
	{
		Id id = GetProjectId(ast);
		if (ast.IsValid(id) && ast.Has<p::FileListenerId>(id))
		{
			fileWatcher.StopListening(ast.Get<p::FileListenerId>(id));
		}
		ast::CloseProject(ast);
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

			if (files::ExistsAsFile(configFile))
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
}    // namespace rift::Editor
