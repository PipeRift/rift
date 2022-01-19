// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CProject.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "Framework/Paths.h"

#include <Files/Files.h>
#include <Files/Paths.h>


namespace Rift::Modules
{
	bool OpenProject(AST::Tree& ast, const Path& path)
	{
		if (path.empty())
		{
			Log::Error("Can't open project: Invalid path");
			return false;
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
			return false;
		}

		ast = AST::Tree{};
		ast.SetStatic<SModules>();
		ast.SetStatic<STypes>();
		LoadSystem::Init(ast);
		TypeSystem::Init(ast);
		FunctionsSystem::Init(ast);

		// Create project node (root module)
		AST::Id projectId = ast.Create();
		ast.Add<CProject, CModule>(projectId);
		ast.Add<CFileRef>(projectId, filePath);

		// Load project module
		TArray<String> strings;
		LoadSystem::LoadFileStrings(ast, projectId, strings);
		LoadSystem::DeserializeModules(ast, projectId, strings);
		return true;
	}

	void CloseProject(AST::Tree& ast)
	{
		ast.Reset();
	}

	AST::Id GetProjectId(const AST::Tree& ast)
	{
		return ast.GetFirstId<CProject>();
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
		const AST::Id projectId = GetProjectId(ast);
		if (projectId != AST::NoId)
		{
			return ast.TryGet<CModule>(projectId);
		}
		return nullptr;
	}

	const CModule* GetProjectModule(const AST::Tree& ast)
	{
		const AST::Id projectId = GetProjectId(ast);
		if (projectId != AST::NoId)
		{
			return ast.TryGet<CModule>(projectId);
		}
		return nullptr;
	}
}    // namespace Rift::Modules
