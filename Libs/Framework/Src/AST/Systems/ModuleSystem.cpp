// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CModule.h"
#include "AST/Systems/ModuleSystem.h"
#include "AST/Tree.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/TypeIterator.h"

#include <Containers/Set.h>


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

    void ScanModuleAssets(AST::Tree& ast)
    {
        TSet<Path> modulePaths;
        auto modules = ast.MakeView<CModule>();
        for (AST::Id entity : modules)
        {
            const CModule& mod = ast.Get<CModule>(entity);
            modulePaths.Insert(mod.path);
        }


		ZoneScopedNC("Find asset files", 0x459bd1);
		for (AST::Id entity : modules)
		{
			CModule& mod = ast.Get<CModule>(entity);
			for (const auto& asset : TypeIterator(mod.path, &modulePaths))
			{
				// allTypes.Add(asset);
			}
		}

		// allTypes.Empty();
	}
}    // namespace Rift::ModuleSystem
