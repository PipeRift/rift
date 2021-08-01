// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/ModuleSystem.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CPendingLoad.h"
#include "AST/Tree.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/ModuleIterator.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeIterator.h"

#include <AST/Linkage.h>
#include <Containers/Set.h>


namespace Rift::ModuleSystem
{
	void ScanSubmodules(AST::Tree& ast)
	{
		ZoneScopedNC("ScanSubmodules", 0x459bd1);

		auto modulesView = ast.MakeView<CModule>();

		AST::Id projectId   = Modules::GetProjectModule(ast);
		auto& projectModule = modulesView.Get<CModule>(projectId);

		for (const auto& modulePath : ModuleIterator(projectModule.path, nullptr))
		{
			const Path folderPath = modulePath.parent_path();
			bool moduleExists     = false;
			for (AST::Id otherId : modulesView)
			{
				auto& other = modulesView.Get<CModule>(otherId);
				if (folderPath == other.path)
				{
					moduleExists = true;
					break;
				}
			}

			if (!moduleExists)
			{
				const AST::Id newModuleId = ast.Create();
				ast.Add<CModule>(newModuleId, false, modulePath);
				ast.Add<CIdentifier>(newModuleId, Name{Paths::ToString(folderPath.filename())});
			}
		}
	}

	void ScanModuleTypes(AST::Tree& ast)
	{
		ZoneScopedNC("ScanModuleTypes", 0x459bd1);

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
