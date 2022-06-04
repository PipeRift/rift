// Copyright 2015-2022 Piperift - All rights reserved

#include "Editor.h"

#include "AST/Systems/FunctionsSystem.h"
#include "Statics/SEditor.h"
#include "Systems/EditorSystem.h"
#include "Utils/FunctionGraph.h"

#include <AST/Statics/SModules.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Systems/LoadSystem.h>
#include <AST/Systems/TransactionSystem.h>
#include <AST/Systems/TypeSystem.h>
#include <AST/Utils/ModuleUtils.h>
#include <Files/Files.h>
#include <Log.h>
#include <Profiler.h>
#include <RiftContext.h>
#include <UI/Inspection.h>
#include <UI/Window.h>


namespace Rift
{
	void RegisterKeyValueInspections()
	{
		UI::RegisterCustomInspection<AST::Id>([](StringView label, void* data, Refl::Type* type) {
			UI::DrawKeyValue(label, data, GetType<ECS::IdTraits<AST::Id>::Entity>());
		});
	}

	int Editor::Run(TPtr<RiftContext> context, StringView projectPath)
	{
		FileWatcher::StartAsync();

		// Setup window
		Log::Info("Initializing editor...");
		if (!Rift::UI::Init())
		{
			Log::Error("Failed to initialize editor");
			return 1;
		}
		Graph::Init();
		RegisterKeyValueInspections();
		Log::Info("Editor is ready");

		// Open a project if a path has been provided
		OpenProject(FromString(projectPath), false);

		while (!UI::WantsToClose())
		{
			frameTime.Tick();

			UI::PreFrame();
			UpdateConfig();

			Tick();
			UI::Render();

			frameTime.PostTick();
			FrameMark;
		}

		Graph::Shutdown();
		UI::Shutdown();
		ShutdownContext();
		return 0;
	}

	Editor::~Editor()
	{
		UI::Shutdown();
	}

	void Editor::Tick()
	{
		if (Modules::HasProject(ast))
		{
			FunctionsSystem::ClearAddedTags(ast);
			TransactionSystem::ClearTags(ast);

			LoadSystem::Run(ast);
			FunctionsSystem::ResolveCallFunctionIds(ast);
			TypeSystem::PropagateVariableTypes(ast);
			TypeSystem::PropagateExpressionTypes(ast);

			EditorSystem::Draw(ast);
			FunctionsSystem::PropagateDirtyIntoCalls(ast);
			FunctionsSystem::PushInvalidPinsBack(ast);
			FunctionsSystem::SyncCallPinsFromFunction(ast);
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
			configFile                 = Pipe::ToString(path);
			ImGui::GetIO().IniFilename = configFile.c_str();
		}
	}

	bool Editor::CreateProject(const Pipe::Path& path, bool closeFirst)
	{
		if (!closeFirst && Modules::HasProject(ast))
		{
			return false;
		}

		if (Modules::CreateProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			SetUIConfigFile(Modules::GetProjectPath(ast) / "Saved/UI.ini");
			return true;
		}
		return false;
	}

	bool Editor::OpenProject(const Pipe::Path& path, bool closeFirst)
	{
		if (!closeFirst && Modules::HasProject(ast))
		{
			return false;
		}

		if (Modules::OpenProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			SetUIConfigFile(Modules::GetProjectPath(ast) / "Saved/UI.ini");
			return true;
		}
		return false;
	}

	void Editor::Close()
	{
		Rift::UI::Close();
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

			if (Files::ExistsAsFile(configFile))
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
}    // namespace Rift
