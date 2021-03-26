// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"

#include <Profiler.h>
#include <UI/Window.h>


namespace Rift
{
	int Editor::Run()
	{
		// Setup window
		if (!Rift::UI::Init(Editor::OnGl3WError))
		{
			return 1;
		}

		while (!UI::WantsToClose())
		{
			frameTime.Tick();

			UI::PreFrame();
			Tick(frameTime.GetDeltaTime());
			UI::Render();

			frameTime.PostTick();
			FrameMark;
		}

		UI::Shutdown();
		return 0;
	}

	Editor::~Editor()
	{
		UI::Shutdown();
	}

	void Editor::Close()
	{
		Rift::UI::Close();
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

	void Editor::Tick(float /*deltaTime*/)
	{
		ZoneScopedN("Tick");
		UpdateConfig();

		rootEditor.Draw();
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

	void Editor::OnGl3WError(int error, const char* description)
	{
		Log::Error("Glfw Error {}: {}", error, description);
	}
}    // namespace Rift
