// Copyright 2015-2022 Piperift - All rights reserved

#include <Pipe/Pipe.h>
#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"


// Benches

int main()
{
	p::Initialize();
	// Run benches
	p::Shutdown();
}
