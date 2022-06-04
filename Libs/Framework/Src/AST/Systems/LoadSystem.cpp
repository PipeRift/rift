// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclFunctionLibrary.h"
#include "AST/Components/CDeclStruct.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CType.h"
#include "AST/Serialization.h"
#include "AST/Statics/SLoadQueue.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/SStringLoad.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/ModuleIterator.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeIterator.h"
#include "AST/Utils/TypeUtils.h"

#include <AST/Utils/Hierarchy.h>
#include <Files/Files.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::LoadSystem
{
	void Init(AST::Tree& ast)
	{
		ast.SetStatic<SLoadQueue>();
		ast.SetStatic<SStringLoad>();
	}

	void Run(AST::Tree& ast)
	{
		LoadSubmodules(ast);
		LoadTypes(ast);
	}

	void LoadSubmodules(AST::Tree& ast)
	{
		TArray<Path> paths;
		ScanSubmodules(ast, paths);

		TArray<AST::Id> idsToLoad;
		CreateModulesFromPaths(ast, paths, idsToLoad);

		TArray<String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeModules(ast, idsToLoad, strings);
	}

	void LoadTypes(AST::Tree& ast)
	{
		TArray<ModuleTypePaths> pathsByModule;
		ScanTypes(ast, pathsByModule);

		TArray<AST::Id> idsToLoad;
		CreateTypesFromPaths(ast, pathsByModule, idsToLoad);

		TArray<String> strings;
		LoadFileStrings(ast, idsToLoad, strings);
		DeserializeTypes(ast, idsToLoad, strings);
	}

	void ScanSubmodules(AST::Tree& ast, TArray<Path>& paths)
	{
		ZoneScoped;

		paths.Empty();

		AST::Id projectId = Modules::GetProjectId(ast);
		auto& projectFile = ast.Get<CFileRef>(projectId);
		for (const auto& modulePath : ModuleIterator(projectFile.path.parent_path(), nullptr))
		{
			paths.Add(modulePath);
		}
	}

	void ScanTypes(AST::Tree& ast, TArray<ModuleTypePaths>& pathsByModule)
	{
		ZoneScoped;

		pathsByModule.Empty(false);

		// Cache module paths in a Set
		TSet<Path> modulePaths;

		TAccess<CModule, CFileRef> access{ast};
		auto modules = ECS::ListAll<CModule>(access);

		modulePaths.Reserve(modules.Size());
		for (AST::Id moduleId : modules)
		{
			const auto& moduleFile = access.Get<const CFileRef>(moduleId);
			modulePaths.Insert(moduleFile.path.parent_path());
		}

		// Find all type files by module
		pathsByModule.Reserve(modules.Size());
		for (AST::Id moduleId : modules)
		{
			const auto& moduleFile = access.Get<const CFileRef>(moduleId);

			auto& paths = pathsByModule.AddRef({moduleId}).paths;
			ZoneScopedN("Iterate module files");
			// Iterate all types ignoring other module paths
			for (const auto& typePath : TypeIterator(moduleFile.path.parent_path(), &modulePaths))
			{
				paths.Add(typePath);
			}
		}
	}

	void CreateModulesFromPaths(AST::Tree& ast, TArray<Path>& paths, TArray<AST::Id>& ids)
	{
		ZoneScoped;

		TAccess<TWrite<CModule>, TWrite<CFileRef>, CProject, TWrite<CChild>, TWrite<CParent>>
		    access{ast};

		// Remove existing module paths
		auto moduleIds = ECS::ListAll<CModule, CFileRef>(access);
		paths.ExcludeIfSwap([&access, &moduleIds](const Pipe::Path& path) {
			bool moduleExists = false;
			for (AST::Id id : moduleIds)
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
			AST::Id id = ids[i];
			access.Add<CModule>(id);
			access.Add<CFileRef>(id, Move(paths[i]));
		}

		// Link modules to the project
		const AST::Id projectId = Modules::GetProjectId(access);
		AST::Hierarchy::AddChildren(access, projectId, ids);
	}

	void CreateTypesFromPaths(
	    AST::Tree& ast, TSpan<ModuleTypePaths> pathsByModule, TArray<AST::Id>& ids)
	{
		ZoneScoped;

		auto* types = ast.TryGetStatic<STypes>();
		if (!types)
		{
			return;
		}

		// Remove existing types
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			modulePaths.paths.ExcludeIfSwap([types, &modulePaths](const Pipe::Path& path) {
				const Name pathName{ToString(path)};

				if (!types->typesByPath.Contains(pathName))
				{
					modulePaths.pathNames.Add(pathName);
					return false;
				}
				return true;
			});
		}

		// Create nodes
		TArray<AST::Id> typeIds;
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			typeIds.Resize(modulePaths.paths.Size());
			ast.Create(typeIds);

			for (i32 i = 0; i < typeIds.Size(); ++i)
			{
				AST::Id id = typeIds[i];
				Path& path = modulePaths.paths[i];

				ast.Add<CFileRef>(id, Move(path));
			}

			for (i32 i = 0; i < typeIds.Size(); ++i)
			{
				Name pathName = modulePaths.pathNames[i];
				types->typesByPath.Insert(pathName, typeIds[i]);
			}
			AST::Hierarchy::AddChildren(ast, modulePaths.moduleId, typeIds);
			ids.Append(typeIds);
		}
	}

	void LoadFileStrings(TAccessRef<CFileRef> access, TSpan<AST::Id> nodes, TArray<String>& strings)
	{
		ZoneScoped;
		strings.Resize(nodes.Size());
		for (i32 i = 0; i < nodes.Size(); ++i)
		{
			if (auto* file = access.TryGet<const CFileRef>(nodes[i])) [[likely]]
			{
				if (!Files::LoadStringFile(file->path, strings[i], 4))
				{
					Log::Error(
					    "File could not be loaded from disk ({})", Pipe::ToString(file->path));
					continue;
				}
			}
		}
	}

	void DeserializeModules(AST::Tree& ast, TSpan<AST::Id> moduleIds, TSpan<String> strings)
	{
		ZoneScoped;
		Check(moduleIds.Size() == strings.Size());

		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			Modules::Deserialize(ast, moduleIds[i], strings[i]);
		}
	}

	void DeserializeTypes(AST::Tree& ast, TSpan<AST::Id> typeIds, TSpan<String> strings)
	{
		ZoneScoped;
		Check(typeIds.Size() == strings.Size());

		for (i32 i = 0; i < typeIds.Size(); ++i)
		{
			Types::Deserialize(ast, typeIds[i], strings[i]);
		}
	}
}    // namespace Rift::LoadSystem
