// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <PCore/Function.h>
#include <PCore/Name.h>
#include <PCore/Platform.h>


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
