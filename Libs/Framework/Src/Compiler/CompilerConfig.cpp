// Copyright 2015-2022 Piperift - All rights reserved

#include "Compiler/CompilerConfig.h"

#include <AST/Utils/ModuleUtils.h>


namespace rift::compiler
{
	void Config::Init(AST::Tree& ast)
	{
		buildPath         = AST::Modules::GetProjectPath(ast) / "Build";
		intermediatesPath = buildPath / "Intermediates";
		binariesPath      = buildPath / buildMode;
	}
}    // namespace rift::compiler
