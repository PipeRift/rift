// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"
#include "Systems/EditorSystem.h"
#include "Uniques/CEditorUnique.h"
#include "Utils/FunctionGraph.h"

#include <AST/Systems/LoadSystem.h>
#include <AST/Systems/ModuleSystem.h>
#include <AST/Uniques/CModulesUnique.h>
#include <AST/Utils/ModuleUtils.h>
#include <Files/Files.h>
#include <Profiler.h>
#include <RiftContext.h>
#include <UI/Window.h>


namespace Rift
{
	int Editor::Run()
	{
		FileWatcher::StartAsync();
		InitializeContext<RiftContext>();

		// Setup window
		if (!Rift::UI::Init())
		{
			return 1;
		}
		Graph::Init();

		while (!UI::WantsToClose())
		{
			frameTime.Tick();

			UI::PreFrame();
			UpdateConfig();

			Tick();
			Draw();
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
		if (!Modules::HasProject(ast))
		{
			return;
		}

		ModuleSystem::ScanSubmodules(ast);
		ModuleSystem::ScanModuleTypes(ast);
		LoadSystem::Run(ast);
	}

	void Editor::Draw()
	{
		EditorSystem::Draw(ast);
	}

	void Editor::SetUIConfigFile(Path path)
	{
		if (UI::GetWindow())
		{
			configFileChanged          = true;
			configFile                 = Paths::ToString(path);
			ImGui::GetIO().IniFilename = configFile.c_str();
		}
	}

	bool Editor::OpenProject(const Path& path, bool closeFirst)
	{
		if (!closeFirst && Modules::HasProject(ast))
		{
			return false;
		}

		AST::Tree newAST = Modules::OpenProject(path);
		if (Modules::HasProject(newAST))
		{
			ast = Move(newAST);
			ast.SetUnique<CEditorUnique>();

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
