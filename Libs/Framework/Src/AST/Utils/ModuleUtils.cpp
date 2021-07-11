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
		const Path fullPath = Paths::ToAbsolute(path);

		auto& modules = ast.GetOrSetUnique<CModulesUnique>();
		if (ast.IsValid(modules.mainModule))
		{
			if (Modules::GetProjectPath(ast) == fullPath)
			{
				Log::Warning("Tried to open a project that is already open.");
			}
			else
			{
				Log::Error("Can't open a project while another is active.");
			}
			return false;
		}
		else if (path.empty())
		{
			Log::Error("Cant open project: Invalid path");
			return false;
		}


		auto manager = AssetManager::Get();
		auto asset   = manager->Load(AssetInfo(fullPath / projectFile)).Cast<ModuleAsset>();
		if (!asset.IsValid())
		{
			Log::Error(
			    "Cant open project: Project file failed to load. Does it exist? Is it corrupted?");
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
