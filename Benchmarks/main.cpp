// Copyright 2015-2022 Piperift - All rights reserved

#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"

#include <Context.h>

// Benches


class NoFileLogContext : public pipe::Context
{
public:
	NoFileLogContext() : pipe::Context(pipe::Path{}) {}
};

int main()
{
	pipe::InitializeContext<NoFileLogContext>();
	// Run benches
}
