// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Log.h"
#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"


// Benches

int main()
{
	p::Log::Init({});    // No folder logging
	// Run benches
	p::Log::Shutdown();
}
