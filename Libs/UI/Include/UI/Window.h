// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <Events/Function.h>
#include <Platform/Platform.h>
#include <Strings/Name.h>


struct GLFWwindow;


namespace Rift::UI
{
	bool Init();
	void Shutdown();

	void PreFrame();
	void Render();

	void Close();
	bool WantsToClose();

	GLFWwindow* GetWindow();
};    // namespace Rift::UI
