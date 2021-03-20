// Copyright 2015-2020 Piperift - All rights reserved

#include <Backends/C/Backend_C.h>
#include <Context.h>
#include <Profiler.h>
#include <Project.h>

#include <chrono>


using namespace Rift;


int main(int argc, char** argv)
{
	ZoneScopedNC("CLI Execution", 0x459bd1);
	Context::Initialize();

	auto project = Create<Project>();
	project->Init(Path("Project"));

	Rift::Backends::CompilerConfig config;
	Rift::Backends::C::Compile(project, config);

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	return 0;
}
