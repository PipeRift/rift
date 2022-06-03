// Copyright 2015-2022 Piperift - All rights reserved

#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"

#include <Context.h>

// Benches


class NoFileLogContext : public Rift::Context
{
public:
	NoFileLogContext() : Rift::Context(Rift::Path{}) {}
};

int main()
{
	Rift::InitializeContext<NoFileLogContext>();
	// Run benches
}
