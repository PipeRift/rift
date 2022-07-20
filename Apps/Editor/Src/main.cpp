// Copyright 2015-2022 Piperift - All rights reserved

#include <CppBackend.h>
#include <Editor.h>
#include <GraphView.h>
#include <LLVMBackend.h>

#include <iostream>


using namespace rift;


#ifndef RUN_AS_CLI
#	define RUN_AS_CLI 1
#endif

int RunEditor(StringView projectPath)
{
	p::Log::Init("Saved/Logs");
	TOwnPtr<rift::Rift> rift = MakeOwned<rift::Rift>();
	rift->AddPlugin<LLVMBackendPlugin>();
	rift->AddPlugin<CPPBackendPlugin>();

	rift->AddPlugin<GraphViewPlugin>();

	const int result = Editor::Get().Run(rift, projectPath);
	p::Log::Shutdown();
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
