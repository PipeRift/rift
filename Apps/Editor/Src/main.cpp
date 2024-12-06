// Copyright 2015-2024 Piperift - All rights reserved

#include <PipeNewDelete.h>
//  Override as first include

#include <ASTModule.h>
#include <Editor.h>
#include <GraphViewModule.h>
#include <MIRBackendModule.h>
#include <Pipe.h>
#include <PipeTime.h>

#include <iostream>


using namespace rift;

#ifndef RUN_AS_CLI
	#define RUN_AS_CLI 1
#endif

int RunEditor(StringView projectPath)
{
	p::Logger logger = p::Logger{.infoCallback = [](StringView msg) {
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Info] {}\n", msg);
		std::cout << text;
	}, .warningCallback = [](StringView msg) {
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Warning] {}\n", msg);
		std::cout << text;
	}, .errorCallback = [](StringView msg) {
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Error] {}\n", msg);
		std::cout << text;
	}};

	p::Initialize(&logger);
	EnableModule<ASTModule>();
	EnableModule<MIRBackendModule>();
	EnableModule<GraphViewModule>();

	const int result = editor::Editor::Get().Run(projectPath);
	p::Shutdown();
	return result;
}

#if PLATFORM_WINDOWS && !RUN_AS_CLI
	#pragma comment(linker, "/subsystem:windows")
	#include <windows.h>
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
