// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include <Context.h>

#if PLATFORM_WINDOWS
#	pragma comment(linker, "/subsystem:windows")
#	include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	Rift::Context::Initialize();
	Window window{};
	return window.Run();
}
#else
int main(int, char**)
{
	Rift::Context::Initialize();
	Window window{};
	return window.Run();
}
#endif
