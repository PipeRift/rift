// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe.h>

#include <cstdlib>


int main(int argc, char* argv[])
{
	p::Initialize();
	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
