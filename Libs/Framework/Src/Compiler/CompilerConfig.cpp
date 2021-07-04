// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Uniques/CModulesUnique.h"
#include "Compiler/CompilerContext.h"
#include "AST/Utils/ModuleUtils.h"


namespace Rift::Compiler
{
	void Config::Init(AST::Tree& ast)
	{
		auto* modules = ast.TryGetUnique<CModulesUnique>();
		if (modules && !modules->HasMainModule())
		{
			buildPath         = Modules::GetProjectPath(ast) / "Build";
			intermediatesPath = buildPath / "Intermediates";
			binariesPath      = buildPath / buildMode;
		}
	}
}    // namespace Rift::Compiler
