// Copyright 2015-2023 Piperift - All rights reserved

#include "Compiler/CompilerConfig.h"

#include <AST/Utils/ModuleUtils.h>


namespace rift
{
	void CompilerConfig::Init(AST::Tree& ast)
	{
		buildPath         = p::JoinPaths(AST::GetProjectPath(ast), "Build");
		intermediatesPath = buildPath / "Intermediates";
		binariesPath      = buildPath / buildMode;
	}
}    // namespace rift
