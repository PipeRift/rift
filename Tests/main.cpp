// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe.h>

#include <ASTModule.h>

#include <cstdlib>


int main(int argc, char* argv[])
{
	p::Initialize();
	rift::EnableModule<rift::ASTModule>();
	int result = p::RunTests(argc, argv);
	rift::DisableModule<rift::ASTModule>();
	p::Shutdown();
	return result;
}
