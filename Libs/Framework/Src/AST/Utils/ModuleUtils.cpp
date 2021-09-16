// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Utils/LoadingUtils.h"
#include "AST/Utils/ModuleUtils.h"
#include "Framework/Paths.h"

#include <Files/Files.h>
#include <Files/Paths.h>



namespace Rift::Modules
{
	AST::Tree OpenProject(const Path& path)
	{
		if (path.empty())
		{
			Log::Error("Cant open project: Invalid path");
			return {};
		}

		Path folderPath;
		Path filePath;
		if (Files::IsFile(path))
		{
			filePath   = Paths::ToAbsolute(path);
			folderPath = filePath.parent_path();
		}
		else
		{
			folderPath = Paths::ToAbsolute(path);
			filePath   = folderPath / moduleFile;
		}

		if (!Files::ExistsAsFile(filePath))
		{
			Log::Error(
			    "Can't open project: Project file failed to load. Does it exist? Is it corrupted?");
			return {};
		}

		AST::Tree ast;
		CModulesUnique& modules = ast.SetUnique<CModulesUnique>();
		LoadSystem::Init(ast);
		TypeSystem::Init(ast);

		// Create root module
		modules.mainModule = ast.Create();
		ast.Add<CModule>(modules.mainModule, true);
		ast.Add<CFileRef>(modules.mainModule, filePath);
		Loading::MarkPendingLoad(ast, modules.mainModule);

		return ast;
	}

	void CloseProject(AST::Tree& ast)
	{
		ast.Reset();
	}

	AST::Id GetProjectId(const AST::Tree& ast)
	{
		if (auto* modules = ast.TryGetUnique<CModulesUnique>())
		{
			if (ast.IsValid(modules->mainModule))
			{
				return modules->mainModule;
			}
		}
		return AST::NoId;
	}

	Name GetProjectName(const AST::Tree& ast)
	{
		AST::Id moduleId = GetProjectId(ast);
		return GetModuleName(ast, moduleId);
	}

	Path GetProjectPath(const AST::Tree& ast)
	{
		return GetModulePath(ast, GetProjectId(ast));
	}

	bool HasProject(const AST::Tree& ast)
	{
		return GetProjectId(ast) != AST::NoId;
	}

	Name GetModuleName(const AST::Tree& ast, AST::Id moduleId)
	{
		if (!ast.IsValid(moduleId))
		{
			return {};
		}

		const auto* identifier = ast.TryGet<CIdentifier>(moduleId);
		if (identifier && !identifier->name.IsNone())
		{
			return identifier->name;
		}

		const auto* file = ast.TryGet<CFileRef>(moduleId);
		if (file && !file->path.empty())
		{
			// Obtain name from project file name
			const String fileName = Paths::ToString(file->path);
			return {Paths::GetFilename(Paths::GetParent(fileName))};    // Folder name
		}
		return {};
	}

	Path GetModulePath(const AST::Tree& ast, AST::Id moduleId)
	{
		if (const auto* file = ast.TryGet<CFileRef>(moduleId))
		{
			return file->path.parent_path();
		}
		return Path{};
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
}    // namespace Rift::Modules
