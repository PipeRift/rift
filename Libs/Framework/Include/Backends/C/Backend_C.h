// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Backends/Backend.h"


namespace Rift::Backends::C
{
	struct CCompilerContext : public CompilerContext
	{
		STRUCT(CCompilerContext, CompilerContext)
	};


	void GenerateCode(const CCompilerContext& config);
	void Compile(Ptr<Project> project, const CompilerConfig& config);
}    // namespace Rift::Backends::C
