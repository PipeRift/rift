// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Context.h"


namespace Rift::Compiler
{
	void Config::Init(AST::Tree& ast)
	{
		buildPath         = Modules::GetProjectPath(ast) / "Build";
		intermediatesPath = buildPath / "Intermediates";
		binariesPath      = buildPath / buildMode;
	}
}    // namespace Rift::Compiler
