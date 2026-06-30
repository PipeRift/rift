// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Compiler/CompilerConfig.h"

#include <AST/Utils/ModuleUtils.h>


namespace rift
{
	void CompilerConfig::Init(ast::Tree& ast)
	{
		buildPath         = p::JoinPaths(ast::GetProjectPath(ast), "Build");
		intermediatesPath = p::JoinPaths(buildPath, "Intermediates");
		binariesPath      = p::JoinPaths(buildPath, "Binaries");
	}
}    // namespace rift
