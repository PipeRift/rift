// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Components/CModule.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"

#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>
#include <PipeECS.h>


namespace rift::ast
{
	static p::TArray<ModuleBinding> gModuleBindings;
	TBroadcast<EntityReader&> gOnReadModulePools;
	TBroadcast<EntityWriter&> gOnWriteModulePools;


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
			p::Error("Can't open project: {}", error);
			return false;
		}

		if (!files::ExistsAsFolder(validatedPath))
		{
			p::Error("Can't open project: Folder doesn't exist");
			return false;
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (!files::ExistsAsFile(filePath))
		{
			p::Error("Can't open project: Folder doesn't contain a '{}' file", moduleFilename);
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
		ast.Add(projectId, CNamespace{p::GetFilename(p::GetParentPath(filePath))});
		ast.Add(projectId, CFileRef{filePath});

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
			p::Error("Can't create module: {}", error);
			return NoId;
		}

		if (!files::ExistsAsFolder(validatedPath))
		{
			files::CreateFolder(validatedPath, true);
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (files::ExistsAsFile(filePath))
		{
			p::Error("Can't create module: Folder already contains a '{}' file", moduleFilename);
			return NoId;
		}

		Id moduleId = ast.Create();
		ast.Add<CModule>(moduleId);
		ast.Add(moduleId, CNamespace{p::GetFilename(p::GetParentPath(filePath))});
		ast.Add(moduleId, CFileRef{filePath});

		p::String data;
		SerializeModule(ast, moduleId, data);
		files::SaveStringFile(filePath, data);
		return moduleId;
	}

	Id GetProjectId(TAccessRef<CProject> access)
	{
		return GetFirstId<CProject>(access);
	}

	Tag GetProjectName(TAccessRef<CProject, CNamespace, CFileRef> access)
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

	Tag GetModuleName(TAccessRef<CNamespace, CFileRef> access, Id moduleId)
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
			return Tag{p::GetFilename(p::GetParentPath(fileName))};    // Folder name
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

	void SerializeModule(ast::Tree& ast, ast::Id id, String& data)
	{
		JsonFormatWriter writer{};
		p::EntityWriter w{writer.GetWriter(), ast};
		w.BeginObject();
		w.SerializeSingleEntity(id, gOnWriteModulePools);

		data = writer.ToString();
	}

	void DeserializeModule(ast::Tree& ast, ast::Id id, const String& data)
	{
		JsonFormatReader formatReader{data};
		if (formatReader.IsValid())
		{
			p::EntityReader r{formatReader, ast};
			r.BeginObject();
			r.SerializeSingleEntity(id, gOnReadModulePools);
		}
	}
	const TBroadcast<EntityReader&>& OnReadModulePools()
	{
		return gOnReadModulePools;
	}
	const TBroadcast<EntityWriter&>& OnWriteModulePools()
	{
		return gOnWriteModulePools;
	}


	void RegisterModuleBinding(ModuleBinding binding)
	{
		gModuleBindings.AddUniqueSorted(Move(binding));
	}
	void UnregisterModuleBinding(p::Tag bindingId)
	{
		gModuleBindings.RemoveSorted(bindingId);
	}
	void AddBindingToModule(ast::Tree& ast, ast::Id id, p::Tag bindingId)
	{
		if (const auto* binding = FindModuleBinding(bindingId))
		{
			ast.AddDefault(binding->tagType->GetId(), id);
		}
	}
	void RemoveBindingFromModule(ast::Tree& ast, ast::Id id, p::Tag bindingId)
	{
		if (const auto* binding = FindModuleBinding(bindingId))
		{
			ast.Remove(binding->tagType->GetId(), id);
		}
	}

	const ModuleBinding* FindModuleBinding(p::Tag id)
	{
		const i32 index = gModuleBindings.FindSortedEqual(id);
		return index != NO_INDEX ? gModuleBindings.Data() + index : nullptr;
	}

	p::TView<const ModuleBinding> GetModuleBindings()
	{
		return gModuleBindings;
	}
}    // namespace rift::ast
