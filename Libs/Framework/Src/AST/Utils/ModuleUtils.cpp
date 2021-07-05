// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CModule.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/ModuleUtils.h"
#include "Framework/Paths.h"

#include <Files/Paths.h>


namespace Rift::Modules
{
	bool OpenProject(AST::Tree& ast, const Path& path)
	{
		auto& modules = ast.GetOrSetUnique<CModulesUnique>();
		if (ast.IsValid(modules.mainModule))
		{
			// Project must be closed first
			return false;
		}

		const Path fullPath = Paths::ToAbsolute(path);

		auto manager = AssetManager::Get();
		auto asset   = manager->Load(AssetInfo(fullPath / projectFile)).Cast<ModuleAsset>();
		if (!asset.IsValid())
		{
			return false;
		}

		// Reset ast
		ast.Reset();

		modules.mainModule = ast.Create();
		ast.Add<CModule>(modules.mainModule, true, asset);

		return true;
	}

	void CloseProject(AST::Tree& ast)
	{
		ast.Reset();
	}

	CModule* GetProjectModule(AST::Tree& ast)
	{
		if (auto* modules = ast.TryGetUnique<CModulesUnique>())
		{
			if (ast.IsValid(modules->mainModule))
			{
				return ast.TryGet<CModule>(modules->mainModule);
			}
		}
		return nullptr;
	}

	const CModule* GetProjectModule(const AST::Tree& ast)
	{
		if (auto* modules = ast.TryGetUnique<CModulesUnique>())
		{
			if (ast.IsValid(modules->mainModule))
			{
				return ast.TryGet<CModule>(modules->mainModule);
			}
		}
		return nullptr;
	}

	Name GetProjectName(const AST::Tree& ast)
	{
		const CModule* module = GetProjectModule(ast);
		return module ? GetModuleName(*module) : Name{};
	}

	const Path& GetProjectPath(const AST::Tree& ast)
	{
		static const Path def{};
		const CModule* module = GetProjectModule(ast);
		return module ? module->path : def;
	}

	Name GetModuleName(const CModule& module)
	{
		if (module.asset.IsValid())
		{
			const auto& asset = module.asset;
			if (!asset->name.IsNone())
			{
				return asset->name;
			}

			// Name is not defined, use the file name
			const String fileName = asset.GetFilename();
			return {Strings::RemoveFromEnd(fileName, Paths::projectExtension)};
		}
		return {};
	}
}    // namespace Rift::Modules
