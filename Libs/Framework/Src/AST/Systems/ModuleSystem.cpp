// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/ModuleSystem.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CPendingLoad.h"
#include "AST/Tree.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/TypeIterator.h"

#include <AST/Linkage.h>
#include <Containers/Set.h>


namespace Rift::ModuleSystem
{
	void ScanModuleTypes(AST::Tree& ast)
	{
		auto* modules = ast.TryGetUnique<CModulesUnique>();
		if (!modules)
		{
			return;
		}

		auto modulesView = ast.MakeView<CModule>();

		// Cache module paths
		TSet<Path> modulePaths;
		modulePaths.Reserve(modulesView.Size());
		for (AST::Id entity : modulesView)
		{
			const CModule& mod = ast.Get<CModule>(entity);
			modulePaths.Insert(mod.path);
		}

		ZoneScopedNC("Find type files", 0x459bd1);
		for (AST::Id moduleId : modulesView)
		{
			TArray<AST::Id> newTypes;
			const CModule& mod = ast.Get<CModule>(moduleId);
			for (const auto& typePath : TypeIterator(mod.path, &modulePaths))
			{
				const Name namePath{Paths::ToString(typePath)};

				if (!modules->typesByPath.Contains(namePath))
				{
					AST::Id unloadedType = ast.Create();
					ast.Add<CPendingLoad>(unloadedType);
					CType& type   = ast.Add<CType>(unloadedType);
					type.path     = typePath;
					type.moduleId = moduleId;

					ast.Add<CIdentifier>(unloadedType, namePath);
					modules->typesByPath.Insert(namePath, unloadedType);
					newTypes.Add(unloadedType);
				}
			}
			AST::Link(ast, moduleId, newTypes);
		}

		// allTypes.Empty();
	}
}    // namespace Rift::ModuleSystem
