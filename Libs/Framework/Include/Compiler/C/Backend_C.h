// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerContext.h"


namespace Rift::Compiler::C
{
	struct Context : public Rift::Compiler::Context
	{
		STRUCT(Context, Rift::Compiler::Context)
	};


	void GenerateCode(const Context& config);
	void Build(TPtr<Project> project, const Config& config);
}    // namespace Rift::Compiler::C
