// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Serialization.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/Paths.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::Modules
{
	bool ValidateProjectPath(Path& path, String& error)
	{
		if (path.empty())
		{
			error = "Path is empty";
			return false;
		}

		if (files::IsFile(path))
		{
			if (path.filename() != moduleFile)
			{
				error = "Path is not a rift module file or a folder";
				return false;
			}
			path = ToAbsolutePath(path).parent_path();
		}
		else
		{
			path = ToAbsolutePath(path);
		}
		return true;
	}

	bool CreateProject(Tree& ast, p::Path path)
	{
		String error;
		if (!ValidateProjectPath(path, error))
		{
			Log::Error("Can't create project: {}", error);
			return false;
		}

		if (!files::ExistsAsFolder(path))
		{
			files::CreateFolder(path, true);
		}

		const p::Path filePath = path / moduleFile;
		if (files::ExistsAsFile(filePath))
		{
			Log::Error("Can't create project: Folder already contains a '{}' file", moduleFile);
			return false;
		}

		JsonFormatWriter writer{};
		writer.GetContext().BeginObject();
		files::SaveStringFile(filePath, writer.ToString());

		return OpenProject(ast, path);
	}

	bool OpenProject(Tree& ast, p::Path path)
	{
		String error;
		if (!ValidateProjectPath(path, error))
		{
			Log::Error("Can't open project: {}", error);
			return false;
		}

		if (!files::ExistsAsFolder(path))
		{
			Log::Error("Can't open project: Folder doesn't exist");
			return false;
		}

		const p::Path filePath = path / moduleFile;
		if (!files::ExistsAsFile(filePath))
		{
			Log::Error("Can't open project: Folder doesn't contain a '{}' file", moduleFile);
			return false;
		}

		ast = Tree{};
		ast.SetStatic<SModules>();
		ast.SetStatic<STypes>();
		LoadSystem::Init(ast);
		TypeSystem::Init(ast);
		FunctionsSystem::Init(ast);

		// Create project node (root module)
		Id projectId = ast.Create();
		ast.Add<CProject, CModule>(projectId);
		ast.Add<CFileRef>(projectId, filePath);

		// Load project module
		TArray<String> strings;
		LoadSystem::LoadFileStrings(ast, projectId, strings);
		LoadSystem::DeserializeModules(ast, projectId, strings);
		return true;
	}

	void CloseProject(Tree& ast)
	{
		ast.Reset();
	}

	Id GetProjectId(TAccessRef<CProject> access)
	{
		return ecs::GetFirst<CProject>(access);
	}

	Name GetProjectName(TAccessRef<CProject, CNamespace, CFileRef> access)
	{
		Id moduleId = GetProjectId(access);
		return GetModuleName(access, moduleId);
	}

	Path GetProjectPath(TAccessRef<CFileRef, CProject> access)
	{
		return GetModulePath(access, GetProjectId(access));
	}

	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access)
	{
		const Id projectId = GetProjectId(access);
		if (projectId != NoId)
		{
			return access.TryGet<CModule>(projectId);
		}
		return nullptr;
	}

	bool HasProject(Tree& ast)
	{
		return GetProjectId(ast) != NoId;
	}

	Name GetModuleName(TAccessRef<CNamespace, CFileRef> access, Id moduleId)
	{
		if (!access.IsValid(moduleId))
		{
			return {};
		}

		const auto* ns = access.TryGet<const CNamespace>(moduleId);
		if (ns && !ns->name.IsNone())
		{
			return ns->name;
		}

		const auto* file = access.TryGet<const CFileRef>(moduleId);
		if (file && !file->path.empty())
		{
			// Obtain name from project file name
			const String fileName = p::ToString(file->path);
			return {GetFilename(GetParentPath(fileName))};    // Folder name
		}
		return {};
	}

	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId)
	{
		if (const auto* file = access.TryGet<const CFileRef>(moduleId))
		{
			return file->path.parent_path();
		}
		return p::Path{};
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		JsonFormatWriter writer{};
		AST::Writer ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ReadWriter common{ct};
		if (auto* ns = ast.TryGet<CNamespace>(id))
		{
			ns->SerializeReflection(common);
		}
		if (auto* module = ast.TryGet<CModule>(id))
		{
			module->SerializeReflection(common);
		}
		data = writer.ToString();
	}

	void Deserialize(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;

		JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		AST::Reader ct{reader, ast};
		ct.BeginObject();
		p::ReadWriter common{ct};
		ast.GetOrAdd<CNamespace>(id).SerializeReflection(common);
		ast.GetOrAdd<CModule>(id).SerializeReflection(common);
	}
}    // namespace rift::Modules
