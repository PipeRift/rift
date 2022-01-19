// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"

#include <iostream>


#ifndef RUN_AS_CLI
#	define RUN_AS_CLI 1
#endif

int RunEditor(Rift::StringView projectPath)
{
	return Rift::Editor::Get().Run(projectPath);
}

#if PLATFORM_WINDOWS && !RUN_AS_CLI
#	pragma comment(linker, "/subsystem:windows")
#	include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return RunEditor(__argc > 1 ? __argv[1] : Rift::StringView{});
}
#else
int main(int argc, char* argv[])
{
	return RunEditor(argc > 1 ? argv[1] : Rift::StringView{});
}
#endif
