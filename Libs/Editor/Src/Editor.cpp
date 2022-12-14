// Copyright 2015-2022 Piperift - All rights reserved

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
#include <Pipe/Core/Profiler.h>
#include <Pipe/Files/Files.h>
#include <UI/Inspection.h>
#include <UI/Window.h>


namespace rift::Editor
{
	void RegisterKeyValueInspections()
	{
		UI::RegisterCustomInspection<AST::Id>([](StringView label, void* data, Type* type) {
			UI::DrawKeyValue(label, data, GetType<ecs::IdTraits<AST::Id>::Entity>());
		});

		UI::RegisterCustomInspection<AST::Namespace>([](StringView label, void* data, Type* type) {
			UI::TableNextRow();
			UI::TableSetColumnIndex(0);
			UI::AlignTextToFramePadding();
			UI::Text(label);
			UI::TableSetColumnIndex(1);
			auto* ns        = static_cast<AST::Namespace*>(data);
			String asString = ns->ToString();
			if (UI::InputText("##value", asString))
			{
				*ns = AST::Namespace{asString};
			}
		});
	}

	int Editor::Run(StringView projectPath)
	{
		FileWatcher::StartAsync();

		// Setup window
		Log::Info("Initializing editor...");
		if (!rift::UI::Init())
		{
			Log::Error("Failed to initialize editor");
			return 1;
		}
		Graph::Init();
		RegisterKeyValueInspections();
		Log::Info("Editor is ready");

		// Open a project if a path has been provided
		OpenProject(p::ToPath(projectPath), false);

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
		return 0;
	}

	Editor::~Editor()
	{
		UI::Shutdown();
	}

	void Editor::Tick()
	{
		if (AST::HasProject(ast))
		{
			AST::FunctionsSystem::ClearAddedTags(ast);
			AST::TransactionSystem::ClearTags(ast);

			AST::LoadSystem::Run(ast);
			AST::FunctionsSystem::ResolveCallFunctionIds(ast);
			AST::TypeSystem::ResolveExprTypeIds(ast);

			EditorSystem::Draw(ast);
			AST::TypeSystem::PropagateVariableTypes(ast);
			AST::FunctionsSystem::PropagateDirtyIntoCalls(ast);
			AST::FunctionsSystem::PushInvalidPinsBack(ast);
			AST::FunctionsSystem::SyncCallPinsFromFunction(ast);
			AST::TypeSystem::PropagateExpressionTypes(ast);
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

	bool Editor::CreateProject(const p::Path& path, bool closeFirst)
	{
		if (!closeFirst && AST::HasProject(ast))
		{
			return false;
		}

		if (AST::CreateProject(ast, path) && AST::OpenProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			SetUIConfigFile(AST::GetProjectPath(ast) / "Saved/UI.ini");
			return true;
		}
		return false;
	}

	bool Editor::OpenProject(const p::Path& path, bool closeFirst)
	{
		if (!closeFirst && AST::HasProject(ast))
		{
			return false;
		}

		if (AST::OpenProject(ast, path))
		{
			ast.SetStatic<SEditor>();
			EditorSystem::Init(ast);
			SetUIConfigFile(AST::GetProjectPath(ast) / "Saved/UI.ini");
			return true;
		}
		return false;
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
