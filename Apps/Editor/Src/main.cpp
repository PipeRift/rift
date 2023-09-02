// Copyright 2015-2023 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
//  Override as first include

#include <Editor.h>
#include <FrameworkModule.h>
#include <GraphViewModule.h>
#include <LLVMBackendModule.h>
#include <MIRBackendModule.h>
#include <Pipe/Pipe.h>

#include <iostream>


using namespace rift;


#ifndef RUN_AS_CLI
#	define RUN_AS_CLI 1
#endif

int RunEditor(StringView projectPath)
{
	p::Initialize("Saved/Logs");
	EnableModule<FrameworkModule>();
	EnableModule<LLVMBackendModule>();
	EnableModule<MIRBackendModule>();
	EnableModule<GraphViewModule>();

	const int result = Editor::Editor::Get().Run(projectPath);
	p::Shutdown();
	return result;
}

#if PLATFORM_WINDOWS && !RUN_AS_CLI
#	pragma comment(linker, "/subsystem:windows")
#	include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return RunEditor(__argc > 1 ? __argv[1] : StringView{});
}
#else
int main(int argc, char* argv[])
{
	return RunEditor(argc > 1 ? argv[1] : StringView{});
}
#endif
