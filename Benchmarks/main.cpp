// Copyright 2015-2022 Piperift - All rights reserved

#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"

#include <Pipe/Core/Context.h>

// Benches


class NoFileLogContext : public p::Context
{
public:
	NoFileLogContext() : p::Context(p::Path{}) {}
};

int main()
{
	p::InitializeContext<NoFileLogContext>();
	// Run benches
}
