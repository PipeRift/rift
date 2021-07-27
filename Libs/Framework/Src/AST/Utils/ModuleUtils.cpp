// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Uniques/CModulesUnique.h"
#include "Framework/Paths.h"

#include <Files/Paths.h>


namespace Rift::Modules
{
	AST::Tree OpenProject(const Path& path)
	{
		const Path fullPath = Paths::ToAbsolute(path);

		if (path.empty())
		{
			Log::Error("Cant open project: Invalid path");
			return {};
		}

		auto manager = AssetManager::Get();
		auto asset   = manager->Load(AssetInfo(fullPath / projectFile)).Cast<ModuleAsset>();
		if (!asset.IsValid())
		{
			Log::Error(
			    "Can't open project: Project file failed to load. Does it exist? Is it corrupted?");
			return {};
		}

		AST::Tree ast;
		CModulesUnique& modules = ast.SetUnique<CModulesUnique>();

		// Create root module
		modules.mainModule = ast.Create();
		ast.Add<CModule>(modules.mainModule, true, asset);
		ast.Add<CIdentifier>(modules.mainModule, Name{Paths::ToString(fullPath.filename())});

		return ast;
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

	bool HasProject(const AST::Tree& ast)
	{
		return GetProjectModule(ast) != nullptr;
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
