// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclStatic.h"
#include "AST/Components/CDeclStruct.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CType.h"
#include "AST/Statics/SLoadQueue.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/SStringLoad.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/ModuleIterator.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeIterator.h"
#include "AST/Utils/TypeUtils.h"
#include "Pipe/Files/Paths.h"

#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::AST::LoadSystem
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
		TArray<String> paths;
		ScanSubmodules(ast, paths);

		TArray<Id> idsToLoad;
		CreateModulesFromPaths(ast, paths, idsToLoad);

		TArray<String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeModules(ast, idsToLoad, strings);
	}

	void LoadTypes(Tree& ast)
	{
		TArray<ModuleTypePaths> pathsByModule;
		ScanTypes(ast, pathsByModule);

		TArray<Id> idsToLoad;
		CreateTypesFromPaths(ast, pathsByModule, idsToLoad);

		TArray<String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeTypes(ast, idsToLoad, strings);
	}

	void ScanSubmodules(Tree& ast, TArray<String>& paths)
	{
		ZoneScoped;

		paths.Clear();

		Id projectId      = GetProjectId(ast);
		auto& projectFile = ast.Get<CFileRef>(projectId);
		for (const auto& modulePath : ModuleIterator(p::GetParentPath(projectFile.path)))
		{
			paths.Add(p::ToString(modulePath));
		}
	}

	void ScanTypes(Tree& ast, TArray<ModuleTypePaths>& pathsByModule)
	{
		ZoneScoped;

		pathsByModule.Clear(false);

		// Cache module paths in a Set
		TSet<Path> modulePaths;

		TAccess<CModule, CFileRef> access{ast};
		auto modules = ecs::ListAll<CModule>(access);

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
			Path path = AST::GetModulePath(access, moduleId);

			auto& paths = pathsByModule.AddRef({moduleId}).paths;
			ZoneScopedN("Iterate module files");
			// Iterate all types ignoring other module paths
			for (const auto& typePath :
			    AST::TypeIterator(path /*TODO: Ignore paths | , &modulePaths*/))
			{
				p::String path = p::ToString(typePath);
				if (p::GetStem(path) != "__module__")    // Ignore module files
				{
					paths.Add(path);
				}
			}
		}
	}

	void CreateModulesFromPaths(Tree& ast, TArray<String>& paths, TArray<Id>& ids)
	{
		ZoneScoped;

		TAccess<TWrite<CModule>, TWrite<CFileRef>, CProject, TWrite<CChild>, TWrite<CParent>>
		    access{ast};

		// Remove existing module paths
		auto moduleIds = ecs::ListAll<CModule, CFileRef>(access);
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

		for (i32 i = 0; i < ids.Size(); ++i)
		{
			Id id = ids[i];
			access.Add<CModule>(id);
			access.Add<CFileRef>(id, Move(paths[i]));
		}

		// Link modules to the project
		const Id projectId = GetProjectId(access);
		p::ecs::AddChildren(access, projectId, ids);
	}

	void CreateTypesFromPaths(Tree& ast, TSpan<ModuleTypePaths> pathsByModule, TArray<Id>& ids)
	{
		ZoneScoped;

		auto* types = ast.TryGetStatic<STypes>();
		if (!types)
		{
			return;
		}

		// Remove already existing types
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			modulePaths.paths.RemoveIfSwap([types, &modulePaths](const p::String& path) {
				return types->typesByPath.Contains(p::Name{path});
			});
		}

		// Create type entities
		TArray<Id> typeIds;
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			typeIds.Resize(modulePaths.paths.Size());
			ast.Create(typeIds);

			for (i32 i = 0; i < typeIds.Size(); ++i)
			{
				const Id id  = typeIds[i];
				String& path = modulePaths.paths[i];

				types->typesByPath.Insert(p::Name{path}, id);
				ast.Add<CFileRef>(id, Move(path));
			}

			p::ecs::AddChildren(ast, modulePaths.moduleId, typeIds);
			ids.Append(typeIds);
		}
	}

	void LoadFileStrings(TAccessRef<CFileRef> access, TSpan<Id> nodes, TArray<String>& strings)
	{
		ZoneScoped;
		strings.Resize(nodes.Size());
		for (i32 i = 0; i < nodes.Size(); ++i)
		{
			if (auto* file = access.TryGet<const CFileRef>(nodes[i])) [[likely]]
			{
				if (!files::LoadStringFile(file->path, strings[i], 4))
				{
					Log::Error("File could not be loaded from disk ({})", file->path);
					continue;
				}
			}
		}
	}

	void DeserializeModules(Tree& ast, TSpan<Id> moduleIds, TSpan<String> strings)
	{
		ZoneScoped;
		Check(moduleIds.Size() == strings.Size());

		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			DeserializeModule(ast, moduleIds[i], strings[i]);
		}
	}

	void DeserializeTypes(Tree& ast, TSpan<Id> typeIds, TSpan<String> strings)
	{
		ZoneScoped;
		Check(typeIds.Size() == strings.Size());

		for (i32 i = 0; i < typeIds.Size(); ++i)
		{
			DeserializeType(ast, typeIds[i], strings[i]);
		}
	}
}    // namespace rift::AST::LoadSystem
