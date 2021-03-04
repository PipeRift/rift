// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <Events/Function.h>
#include <Platform/Platform.h>


struct GLFWwindow;


namespace Rift::UI
{
	using ErrorFunc = void (*)(int, const char*);

	bool Init(ErrorFunc onError);
	void Shutdown();

	void PreFrame();
	void Render();

	void Close();
	bool WantsToClose();

	GLFWwindow* GetWindow();
};    // namespace Rift::UI
