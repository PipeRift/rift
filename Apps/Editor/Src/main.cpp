// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"

#ifndef RUN_AS_CLI
#	define RUN_AS_CLI 1
#endif

int RunEditor()
{
	return Rift::Editor::Get().Run();
}

#if PLATFORM_WINDOWS && !RUN_AS_CLI
#	pragma comment(linker, "/subsystem:windows")
#	include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return RunEditor();
}
#else
int main(int, char**)
{
	return RunEditor();
}
#endif
