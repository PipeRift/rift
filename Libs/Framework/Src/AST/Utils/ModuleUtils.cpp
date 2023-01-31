// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Components/CModule.h"
#include "AST/Components/CRiftModule.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/Paths.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/ECS/Serialization.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::AST
{
	auto moduleComponents = [](auto& rw) {
		rw.template SerializeComponents<CNamespace, CModule, CRiftModule>();
	};

	bool ValidateModulePath(p::String& path, p::String& error)
	{
		if (path.empty())
		{
			error = "Path is empty";
			return false;
		}

		if (files::IsFile(path))
		{
			if (p::GetFilename(path) != moduleFilename)
			{
				error = "Path is not a rift module file or a folder";
				return false;
			}
			path = p::GetParentPath(p::ToAbsolutePath(path));
		}
		else
		{
			path = p::String(p::ToAbsolutePath(path));
		}
		return true;
	}

	bool CreateProject(Tree& ast, p::StringView path)
	{
		return CreateModule(ast, path) != NoId;
	}

	bool OpenProject(Tree& ast, p::StringView path)
	{
		String validatedPath{path};
		String error;
		if (!ValidateModulePath(validatedPath, error))
		{
			Log::Error("Can't open project: {}", error);
			return false;
		}

		if (!files::ExistsAsFolder(validatedPath))
		{
			Log::Error("Can't open project: Folder doesn't exist");
			return false;
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (!files::ExistsAsFile(filePath))
		{
			Log::Error("Can't open project: Folder doesn't contain a '{}' file", moduleFilename);
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

	Id CreateModule(Tree& ast, p::StringView path)
	{
		String validatedPath{path};

		String error;
		if (!ValidateModulePath(validatedPath, error))
		{
			Log::Error("Can't create module: {}", error);
			return NoId;
		}

		if (!files::ExistsAsFolder(validatedPath))
		{
			files::CreateFolder(validatedPath, true);
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (files::ExistsAsFile(filePath))
		{
			Log::Error("Can't create module: Folder already contains a '{}' file", moduleFilename);
			return NoId;
		}

		Id moduleId = ast.Create();
		ast.Add<CModule, CRiftModule>(moduleId);

		p::String data;
		SerializeModule(ast, moduleId, data);
		files::SaveStringFile(filePath, data);
		return moduleId;
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

	p::StringView GetProjectPath(TAccessRef<CFileRef, CProject> access)
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
			return Name{p::GetFilename(p::GetParentPath(fileName))};    // Folder name
		}
		return {};
	}

	p::StringView GetModulePath(TAccessRef<CFileRef> access, Id moduleId)
	{
		if (const auto* file = access.TryGet<const CFileRef>(moduleId))
		{
			return p::GetParentPath(file->path);
		}
		return {};
	}

	void SerializeModule(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;
		JsonFormatWriter writer{};
		p::ecs::EntityWriter w{writer.GetWriter(), ast};
		w.BeginObject();
		w.SerializeSingleEntity(id, moduleComponents);
		data = writer.ToString();
	}

	void DeserializeModule(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;
		JsonFormatReader formatReader{data};
		if (formatReader.IsValid())
		{
			p::ecs::EntityReader r{formatReader, ast};
			r.BeginObject();
			r.SerializeSingleEntity(id, moduleComponents);
		}
	}
}    // namespace rift::AST
