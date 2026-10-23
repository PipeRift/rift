// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include <Pipe/Core/Function.h>
#include <Pipe/Core/Tag.h>
#include <PipePlatform.h>


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

	void SetWindowIcon();
};    // namespace rift::UI
