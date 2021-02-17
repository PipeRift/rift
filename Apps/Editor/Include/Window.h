// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Editors/RootEditor.h"

#include <Misc/Time.h>


using namespace Rift;


class Window
{
	struct GLFWwindow* window = nullptr;
	FrameTime frameTime;

	RootEditor rootEditor;

	bool configFileChanged = false;
	String configFile;


public:
	Window()
	{
		frameTime.SetFPSCap(60);
	}

	~Window();

	int Run();

	void Close();

	void SetUIConfigFile(Path path);

	static Window& Get()
	{
		static Window instance{};
		return instance;
	}

	RootEditor& GetRootEditor()
	{
		return rootEditor;
	}

protected:
	void Tick(float deltaTime);

	void UpdateConfig();

	static void OnGl3WError(int error, const char* description);
};
