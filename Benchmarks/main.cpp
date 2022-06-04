// Copyright 2015-2022 Piperift - All rights reserved

#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"

#include <Context.h>

// Benches


class NoFileLogContext : public Pipe::Context
{
public:
	NoFileLogContext() : Pipe::Context(Pipe::Path{}) {}
};

int main()
{
	Pipe::InitializeContext<NoFileLogContext>();
	// Run benches
}
