// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/ModuleSystem.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CPendingLoad.h"
#include "AST/Tree.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/LoadingUtils.h"
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

		auto modulesView = ast.MakeView<CModule, CFileRef>();

		AST::Id projectId = Modules::GetProjectModule(ast);
		auto& projectFile = modulesView.Get<CFileRef>(projectId);

		TArray<AST::Id> newModules;
		for (const auto& modulePath : ModuleIterator(projectFile.path, nullptr))
		{
			const Path folderPath = modulePath.parent_path();
			bool moduleExists     = false;
			for (AST::Id otherId : modulesView)
			{
				auto& otherFile = modulesView.Get<CFileRef>(otherId);
				if (folderPath == otherFile.path)
				{
					moduleExists = true;
					break;
				}
			}

			if (!moduleExists)
			{
				const AST::Id id = ast.Create();
				ast.Add<CModule>(id, false);
				ast.Add<CFileRef>(id, modulePath.parent_path());
				// ast.Add<CIdentifier>(id, Name{Paths::ToString(folderPath.filename())});
				newModules.Add(id);
			}
		}

		Loading::MarkPendingLoad(ast, newModules);
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
		modulePaths.Reserve(u32(modulesView.Size()));
		for (AST::Id moduleId : modulesView)
		{
			const CFileRef& moduleFile = ast.Get<CFileRef>(moduleId);
			modulePaths.Insert(moduleFile.path);
		}

		for (AST::Id moduleId : modulesView)
		{
			const CFileRef& moduleFile = ast.Get<CFileRef>(moduleId);

			TArray<AST::Id> newTypes;
			for (const auto& typePath : TypeIterator(moduleFile.path, &modulePaths))
			{
				const Name namePath{Paths::ToString(typePath)};

				if (!modules->typesByPath.Contains(namePath))
				{
					AST::Id unloadedType = ast.Create();
					CType& type          = ast.Add<CType>(unloadedType);
					type.moduleId        = moduleId;

					ast.Add<CFileRef>(unloadedType, typePath);
					ast.Add<CIdentifier>(unloadedType, namePath);
					newTypes.Add(unloadedType);

					modules->typesByPath.Insert(namePath, unloadedType);
				}
			}
			AST::Link(ast, moduleId, newTypes);
			Loading::MarkPendingLoad(ast, newTypes);
		}

		// allTypes.Empty();
	}
}    // namespace Rift::ModuleSystem
