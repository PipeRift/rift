// Copyright 2015-2022 Piperift - All rights reserved

#include "Editor.h"

// Backends
#include <CppBackend.h>
#include <LLVMBackend.h>

// Views
#include <GraphView.h>

#include <iostream>


using namespace rift;


#ifndef RUN_AS_CLI
#	define RUN_AS_CLI 1
#endif

int RunEditor(StringView projectPath)
{
	auto context = InitializeContext<RiftContext>();
	context->AddPlugin<LLVMBackendPlugin>();
	context->AddPlugin<CPPBackendPlugin>();

	context->AddPlugin<GraphViewPlugin>();

	return Editor::Get().Run(context, projectPath);
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
