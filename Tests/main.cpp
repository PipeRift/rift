// Copyright 2015-2023 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
// P_OVERRIDE_NEW_DELETE

#include <bandit/bandit.h>
#include <Pipe/Pipe.h>


int main(int argc, char* argv[])
{
	p::Initialize();
	int result = bandit::run(argc, argv);
	p::Shutdown();
}
