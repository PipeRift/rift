// Copyright 2015-2020 Piperift - All rights reserved

#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <Context.h>
#include <Module.h>
#include <Profiler.h>
#include <RiftContext.h>

#include <chrono>


using namespace Rift;


int main(int argc, char** argv)
{
	ZoneScopedNC("CLI Execution", 0x459bd1);
	auto context = InitializeContext<RiftContext>();

	AST::Tree ast;
	Modules::OpenProject(ast, Path("Project"));

	auto project = context->OpenProject(Path("Project"));

	Rift::Compiler::Config config;
	Rift::Compiler::Build(ast, config, Rift::Compiler::EBackend::Cpp);

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	ShutdownContext();
	return 0;
}
