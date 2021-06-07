// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerContext.h"


namespace Rift::Compiler::Cpp
{
	struct Context : public Rift::Compiler::Context
	{
		STRUCT(Context, Rift::Compiler::Context)
	};

	void Build(TPtr<Project> project, const Config& config);
}    // namespace Rift::Compiler::Cpp
