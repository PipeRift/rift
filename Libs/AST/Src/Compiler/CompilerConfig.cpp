// Copyright 2015-2023 Piperift - All rights reserved

#include "Compiler/CompilerConfig.h"

#include <AST/Utils/ModuleUtils.h>


namespace rift
{
	void CompilerConfig::Init(ast::Tree& ast)
	{
		buildPath         = p::JoinPaths(ast::GetProjectPath(ast), "Build");
		intermediatesPath = buildPath / "Intermediates";
		binariesPath      = buildPath / "Binaries";
	}
}    // namespace rift
