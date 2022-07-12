// Copyright 2015-2022 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
PIPE_OVERRIDE_NEW_DELETE

#include <bandit/bandit.h>
#include <Pipe/Core/Context.h>


class NoFileLogContext : public p::Context
{
public:
	NoFileLogContext() : p::Context(p::Path{}) {}
};


int main(int argc, char* argv[])
{
	p::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	p::ShutdownContext();
}
