// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"


int RunEditor()
{
	return Rift::Editor::Get().Run();
}

#if PLATFORM_WINDOWS
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
