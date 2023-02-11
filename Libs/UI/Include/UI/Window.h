// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Pipe/Core/Function.h>
#include <Pipe/Core/Platform.h>
#include <Pipe/Core/Tag.h>



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
