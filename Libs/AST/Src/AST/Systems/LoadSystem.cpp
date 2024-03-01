// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Statics/SLoadQueue.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/SStringLoad.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/ModuleFileIterator.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeIterator.h"
#include "AST/Utils/TypeUtils.h"
#include "Pipe/Files/Paths.h"

#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::ast::LoadSystem
{
	void Init(Tree& ast)
	{
		ast.SetStatic<SLoadQueue>();
		ast.SetStatic<SStringLoad>();
	}

	void Run(Tree& ast)
	{
		LoadSubmodules(ast);
		LoadTypes(ast);
	}

	void LoadSubmodules(Tree& ast)
	{
		p::TArray<p::String> paths;
		ScanSubmodules(ast, paths);

		p::TArray<Id> idsToLoad;
		CreateModulesFromPaths(ast, paths, idsToLoad);

		p::TArray<p::String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeModules(ast, idsToLoad, strings);
	}

	void LoadTypes(Tree& ast)
	{
		p::TArray<ModuleTypePaths> pathsByModule;
		ScanTypes(ast, pathsByModule);

		p::TArray<Id> idsToLoad;
		CreateTypesFromPaths(ast, pathsByModule, idsToLoad);

		p::TArray<p::String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeTypes(ast, idsToLoad, strings);
	}

	void ScanSubmodules(Tree& ast, p::TArray<p::String>& paths)
	{
		paths.Clear();

		Id projectId      = GetProjectId(ast);
		auto& projectFile = ast.Get<CFileRef>(projectId);
		for (const auto& modulePath : ModuleFileIterator(p::GetParentPath(projectFile.path)))
		{
			paths.Add(p::ToString(modulePath));
		}
	}

	void ScanTypes(Tree& ast, p::TArray<ModuleTypePaths>& pathsByModule)
	{
		pathsByModule.Clear(false);

		// Cache module paths in a Set
		p::TSet<p::StringView> modulePaths;

		p::TAccess<CModule, CFileRef> access{ast};
		auto modules = FindAllIdsWith<CModule, CFileRef>(access);

		modulePaths.Reserve(modules.Size());
		for (Id moduleId : modules)
		{
			const auto& moduleFile = access.Get<const CFileRef>(moduleId);
			modulePaths.Insert(p::GetParentPath(moduleFile.path));
		}

		// Find all type files by module
		pathsByModule.Reserve(modules.Size());
		for (Id moduleId : modules)
		{
			p::StringView path = ast::GetModulePath(access, moduleId);

			auto& paths = pathsByModule.AddRef({moduleId}).paths;
			// Iterate all types ignoring other module paths
			for (const auto& typePath :
			    ast::TypeIterator(path /*TODO: Ignore paths | , &modulePaths*/))
			{
				p::String path = p::ToString(typePath);
				if (p::GetStem(path) != "__module__")    // Ignore module files
				{
					paths.Add(path);
				}
			}
		}
	}

	void CreateModulesFromPaths(Tree& ast, p::TArray<p::String>& paths, p::TArray<Id>& ids)
	{
		p::TAccess<p::TWrite<CModule>, p::TWrite<CFileRef>, p::TWrite<CNamespace>, CProject,
		    p::TWrite<CChild>, p::TWrite<CParent>>
		    access{ast};

		// Remove existing module paths
		auto moduleIds = p::FindAllIdsWith<CModule, CFileRef>(access);
		paths.RemoveIfSwap([&access, &moduleIds](const p::String& path) {
			bool moduleExists = false;
			for (Id id : moduleIds)
			{
				if (path == access.Get<const CFileRef>(id).path)
				{
					moduleExists = true;
					break;
				}
			}
			return moduleExists;
		});

		ids.Resize(paths.Size());
		access.GetContext().Create(ids);

		for (p::i32 i = 0; i < ids.Size(); ++i)
		{
			Id id           = ids[i];
			p::String& path = paths[i];
			access.Add<CModule>(id);
			access.Add(id, CNamespace{p::GetFilename(p::GetParentPath(path))});
			access.Add(id, CFileRef{p::Move(path)});
		}

		// Link modules to the project
		const Id projectId = GetProjectId(access);
		p::AttachId(access, projectId, ids);
	}

	void CreateTypesFromPaths(
	    Tree& ast, p::TView<ModuleTypePaths> pathsByModule, p::TArray<Id>& ids)
	{
		auto* types = ast.TryGetStatic<STypes>();
		if (!types)
		{
			return;
		}

		// Remove already existing types
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			modulePaths.paths.RemoveIfSwap([types, &modulePaths](const p::String& path) {
				return types->typesByPath.Contains(p::Tag{path});
			});
		}

		// Create type entities
		p::TArray<Id> typeIds;
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			typeIds.Resize(modulePaths.paths.Size());
			ast.Create(typeIds);

			for (p::i32 i = 0; i < typeIds.Size(); ++i)
			{
				const Id id     = typeIds[i];
				p::String& path = modulePaths.paths[i];

				types->typesByPath.Insert(p::Tag{path}, id);
				ast.Add(id, CFileRef{p::Move(path)});
			}

			p::AttachId(ast, modulePaths.moduleId, typeIds);
			ids.Append(typeIds);
		}
	}

	void LoadFileStrings(
	    p::TAccessRef<CFileRef> access, p::TView<Id> nodes, p::TArray<p::String>& strings)
	{
		strings.Resize(nodes.Size());
		for (p::i32 i = 0; i < nodes.Size(); ++i)
		{
			if (auto* file = access.TryGet<const CFileRef>(nodes[i])) [[likely]]
			{
				if (!p::LoadStringFile(file->path, strings[i], 4))
				{
					p::Error("File could not be loaded from disk ({})", file->path);
					continue;
				}
			}
		}
	}

	void DeserializeModules(Tree& ast, p::TView<Id> moduleIds, p::TView<p::String> strings)
	{
		Check(moduleIds.Size() == strings.Size());

		for (p::i32 i = 0; i < moduleIds.Size(); ++i)
		{
			DeserializeModule(ast, moduleIds[i], strings[i]);
		}
	}

	void DeserializeTypes(Tree& ast, p::TView<Id> typeIds, p::TView<p::String> strings)
	{
		Check(typeIds.Size() == strings.Size());

		for (p::i32 i = 0; i < typeIds.Size(); ++i)
		{
			DeserializeType(ast, typeIds[i], strings[i]);
		}
	}
}    // namespace rift::ast::LoadSystem
