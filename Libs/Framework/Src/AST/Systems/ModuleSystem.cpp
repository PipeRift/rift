// Copyright 2015-2020 Piperift - All rights reserved
#include "AST/Systems/ModuleSystem.h"
#include "AST/Tree.h"
#include "AST/Uniques/CModulesUnique.h"


namespace Rift::ModuleSystem
{
	void Init(AST::Tree& ast)
	{
		if (ast.TryGetUnique<CModulesUnique>() == nullptr)
		{
			ast.SetUnique<CModulesUnique>();
		}
	}

	void Run(AST::Tree& ast)
	{
		auto& assets = ast.GetUnique<CModulesUnique>();
	}

	void CacheDependencies(AST::Tree& ast) {}

	void LoadAllModules(AST::Tree& ast)
	{
		auto& modules = ast.GetUnique<CModulesUnique>();

		if (!modules.main.Load())
		{
			return;
		}
	}
}    // namespace Rift::ModuleSystem
