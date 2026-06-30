// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Utils/ModuleUtils.h"

#include "AST/Components/CModule.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"

#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <PipeECS.h>


namespace rift::ast
{
	static p::TArray<ModuleBinding> gModuleBindings;
	p::TBroadcast<p::EntityReader&> gOnReadModulePools;
	p::TBroadcast<p::EntityWriter&> gOnWriteModulePools;


	bool ValidateModulePath(p::String& path, p::String& error)
	{
		if (path.empty())
		{
			error = "Path is empty";
			return false;
		}

		if (p::IsFile(path))
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
		p::String validatedPath{path};
		p::String error;
		if (!ValidateModulePath(validatedPath, error))
		{
			p::Error("Can't open project: {}", error);
			return false;
		}

		if (!p::ExistsAsFolder(validatedPath))
		{
			p::Error("Can't open project: Folder doesn't exist");
			return false;
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (!p::ExistsAsFile(filePath))
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
		Id projectId = p::AddId(ast);
		ast.Add<CProject, CModule>(projectId);
		ast.Add(projectId, CNamespace{p::GetFilename(p::GetParentPath(filePath))});
		ast.Add(projectId, CFileRef{filePath});

		// Load project module
		p::TArray<p::String> strings;
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
		p::String validatedPath{path};

		p::String error;
		if (!ValidateModulePath(validatedPath, error))
		{
			p::Error("Can't create module: {}", error);
			return NoId;
		}

		if (!p::ExistsAsFolder(validatedPath))
		{
			p::CreateFolder(validatedPath, true);
		}

		const p::String filePath = p::JoinPaths(validatedPath, moduleFilename);
		if (p::ExistsAsFile(filePath))
		{
			p::Error("Can't create module: Folder already contains a '{}' file", moduleFilename);
			return NoId;
		}

		Id moduleId = p::AddId(ast);
		ast.Add<CModule>(moduleId);
		ast.Add(moduleId, CNamespace{p::GetFilename(p::GetParentPath(filePath))});
		ast.Add(moduleId, CFileRef{filePath});

		p::String data;
		SerializeModule(ast, moduleId, data);
		p::SaveStringFile(filePath, data);
		return moduleId;
	}

	Id GetProjectId(p::TIdScopeRef<CProject> scope)
	{
		return GetFirstIdWith<CProject>(scope);
	}

	p::Tag GetProjectName(p::TIdScopeRef<CProject, CNamespace, CFileRef> scope)
	{
		Id moduleId = GetProjectId(scope);
		return GetModuleName(scope, moduleId);
	}

	p::StringView GetProjectPath(p::TIdScopeRef<CFileRef, CProject> scope)
	{
		return GetModulePath(scope, GetProjectId(scope));
	}

	CModule* GetProjectModule(p::TIdScopeRef<p::Writes<CModule>, CProject> scope)
	{
		const Id projectId = GetProjectId(scope);
		if (projectId != NoId)
		{
			return scope.TryGet<CModule>(projectId);
		}
		return nullptr;
	}

	bool HasProject(Tree& ast)
	{
		return GetProjectId(ast) != NoId;
	}

	p::Tag GetModuleName(p::TIdScopeRef<CNamespace, CFileRef> scope, Id moduleId)
	{
		if (!scope.IsValid(moduleId))
		{
			return {};
		}

		const auto* ns = scope.TryGet<const CNamespace>(moduleId);
		if (ns && !ns->name.IsNone())
		{
			return ns->name;
		}

		const auto* file = scope.TryGet<const CFileRef>(moduleId);
		if (file && !file->path.empty())
		{
			// Obtain name from project file name
			const p::String fileName = p::ToString(file->path);
			return p::Tag{p::GetFilename(p::GetParentPath(fileName))};    // Folder name
		}
		return {};
	}

	p::StringView GetModulePath(p::TIdScopeRef<CFileRef> scope, Id moduleId)
	{
		if (const auto* file = scope.TryGet<const CFileRef>(moduleId))
		{
			return p::GetParentPath(file->path);
		}
		return {};
	}

	void SerializeModule(ast::Tree& ast, ast::Id id, p::String& data)
	{
		p::JsonFormatWriter writer{};
		p::EntityWriter w{writer.GetWriter(), ast};
		w.BeginObject();
		w.SerializeSingleEntity(id, gOnWriteModulePools);

		data = writer.ToString();
	}

	void DeserializeModule(ast::Tree& ast, ast::Id id, const p::String& data)
	{
		p::JsonFormatReader formatReader{data};
		if (formatReader.IsValid())
		{
			p::EntityReader r{formatReader, ast};
			r.BeginObject();
			r.SerializeSingleEntity(id, gOnReadModulePools);
		}
	}
	const p::TBroadcast<p::EntityReader&>& OnReadModulePools()
	{
		return gOnReadModulePools;
	}
	const p::TBroadcast<p::EntityWriter&>& OnWriteModulePools()
	{
		return gOnWriteModulePools;
	}


	void RegisterModuleBinding(ModuleBinding binding)
	{
		gModuleBindings.AddUniqueSorted(p::Move(binding));
	}
	void UnregisterModuleBinding(p::Tag bindingId)
	{
		gModuleBindings.RemoveSorted(bindingId);
	}
	void AddBindingToModule(ast::Tree& ast, ast::Id id, p::Tag bindingId)
	{
		if (const auto* binding = FindModuleBinding(bindingId))
		{
			ast.AddByTypeId(binding->tagType, id);
		}
	}
	void RemoveBindingFromModule(ast::Tree& ast, ast::Id id, p::Tag bindingId)
	{
		if (const auto* binding = FindModuleBinding(bindingId))
		{
			ast.RemoveByTypeId(binding->tagType, id);
		}
	}

	const ModuleBinding* FindModuleBinding(p::Tag id)
	{
		const p::i32 index = gModuleBindings.FindSorted(id);
		return index != p::NO_INDEX ? gModuleBindings.Data() + index : nullptr;
	}

	p::TView<const ModuleBinding> GetModuleBindings()
	{
		return gModuleBindings;
	}
}    // namespace rift::ast
