// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Core/Function.h>
#include <Core/Name.h>
#include <Core/Platform.h>


struct GLFWwindow;


namespace rift::UI
{
	bool Init();
	void Shutdown();

	void PreFrame();
	void Render();

	void Close();
	bool WantsToClose();

	GLFWwindow* GetWindow();
};    // namespace rift::UI
