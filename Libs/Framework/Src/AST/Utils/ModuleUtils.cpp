// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/ModuleUtils.h"
#include "Framework/Paths.h"

#include <Files/Paths.h>


namespace Rift::Modules
{
	void OpenProject(AST::Tree& ast, const Path& path)
	{
		// Reset ast
		ast.Reset();

		Path fullPath = Paths::ToAbsolute(path);
		auto& modules = ast.SetUnique<CModulesUnique>();

		auto manager = AssetManager::Get();
		modules.main = manager->Load(AssetInfo(fullPath / projectFile)).Cast<ModuleAsset>();

		modules.dependencies.Empty();
	}

	Name GetProjectName(const AST::Tree& ast)
	{
		auto* modules = ast.TryGetUnique<CModulesUnique>();
		return modules ? GetModuleName(modules->main) : Name{};
	}

	Path GetProjectPath(const CModulesUnique& modules)
	{
		return Paths::FromString(modules.main.GetStrPath());
	}

	Name GetModuleName(TAssetPtr<ModuleAsset> module)
	{
		if (module.IsValid())
		{
			if (!module->name.IsNone())
			{
				return module->name;
			}

			// Name is not defined, use the file name
			const String fileName = module.GetFilename();
			return {Strings::RemoveFromEnd(fileName, Paths::projectExtension)};
		}
		return {};
	}
}    // namespace Rift::Modules
