// Copyright 2015-2020 Piperift - All rights reserved

#include <Backends/C/Backend_C.h>
#include <Context.h>
#include <Profiler.h>
#include <Project.h>

#include <chrono>


using namespace VCLang;
using Backend_C = VCLang::Backends::C::Backend_C;


int main(int argc, char** argv)
{
	ScopedZone("CLI Execution", 459bd1);
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();
	project->Init(Path("Project"));

	GlobalPtr<Backend_C> backend = Create<Backend_C>();
	backend->SetProject(project);
	backend->Compile();

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	return 0;
}
