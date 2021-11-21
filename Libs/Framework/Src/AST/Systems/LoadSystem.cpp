// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CType.h"
#include "AST/Serialization.h"
#include "AST/Uniques/CLoadQueueUnique.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Uniques/CStringLoadUnique.h"
#include "AST/Uniques/CTypesUnique.h"
#include "AST/Utils/ModuleIterator.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeIterator.h"
#include "AST/Utils/TypeUtils.h"

#include <AST/Linkage.h>
#include <Files/Files.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::LoadSystem
{
	void Init(AST::Tree& ast)
	{
		ast.SetStatic<CLoadQueueUnique>();
		ast.SetStatic<CStringLoadUnique>();
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
		ZoneScopedNC("ScanSubmodules", 0x459bd1);

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
		ZoneScopedNC("ScanTypes", 0x459bd1);

		pathsByModule.Empty(false);
		auto modulesView = ast.Query<CModule>();

		// Cache module paths in a Set
		TSet<Path> modulePaths;
		modulePaths.Reserve(u32(modulesView.Size()));
		for (AST::Id moduleId : modulesView)
		{
			const CFileRef& moduleFile = ast.Get<CFileRef>(moduleId);
			modulePaths.Insert(moduleFile.path.parent_path());
		}

		// Find all type files by module
		pathsByModule.Reserve(u32(modulesView.Size()));
		for (AST::Id moduleId : modulesView)
		{
			const CFileRef& moduleFile = ast.Get<CFileRef>(moduleId);

			auto& paths = pathsByModule.AddRef({moduleId}).paths;
			// Iterate all types ignoring other module paths
			for (const auto& typePath : TypeIterator(moduleFile.path.parent_path(), &modulePaths))
			{
				paths.Add(typePath);
			}
		}
	}

	void CreateModulesFromPaths(AST::Tree& ast, TArray<Path>& paths, TArray<AST::Id>& ids)
	{
		ZoneScopedNC("CreateModulesFromPaths", 0x459bd1);

		// Remove existing Modules
		auto modulesView = ast.Query<CModule, CFileRef>();
		paths.RemoveIfSwap([&modulesView](const Path& path) {
			bool moduleExists = false;
			for (AST::Id otherId : modulesView)
			{
				auto& otherFile = modulesView.Get<CFileRef>(otherId);
				if (path == otherFile.path)
				{
					moduleExists = true;
					break;
				}
			}
			return moduleExists;
		});


		ids.Resize(paths.Size());
		ast.Create(ids);

		for (i32 i = 0; i < ids.Size(); ++i)
		{
			AST::Id id = ids[i];
			ast.Add<CModule>(id);
			ast.Add<CFileRef>(id, Move(paths[i]));
		}

		// Link modules to the project
		const AST::Id projectId = Modules::GetProjectId(ast);
		AST::Link(ast, projectId, ids);
	}

	void CreateTypesFromPaths(
	    AST::Tree& ast, TArrayView<ModuleTypePaths> pathsByModule, TArray<AST::Id>& ids)
	{
		ZoneScopedNC("CreateTypesFromPaths", 0x459bd1);

		auto* types = ast.TryGetStatic<CTypesUnique>();
		if (!types)
		{
			return;
		}

		// Remove existing types
		for (ModuleTypePaths& modulePaths : pathsByModule)
		{
			modulePaths.paths.RemoveIfSwap([types, &modulePaths](const Path& path) {
				const Name pathName{Paths::ToString(path)};

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

				ast.Add<CType>(id);
				ast.Add<CFileRef>(id, Move(path));
			}

			for (i32 i = 0; i < typeIds.Size(); ++i)
			{
				Name pathName = modulePaths.pathNames[i];
				types->typesByPath.Insert(pathName, typeIds[i]);
			}
			AST::Link(ast, modulePaths.moduleId, typeIds);
			ids.Append(typeIds);
		}
	}

	void LoadFileStrings(AST::Tree& ast, TArrayView<AST::Id> nodes, TArray<String>& strings)
	{
		ZoneScopedNC("LoadFileStrings", 0x459bd1);

		strings.Resize(nodes.Size());

		auto filesView = ast.Query<CFileRef>();
		for (i32 i = 0; i < nodes.Size(); ++i)
		{
			if (auto* file = filesView.TryGet<CFileRef>(nodes[i])) [[likely]]
			{
				if (!Files::LoadStringFile(file->path, strings[i], 4))
				{
					Log::Error(
					    "File could not be loaded from disk ({})", Paths::ToString(file->path));
					continue;
				}
			}
		}
	}

	void DeserializeModules(
	    AST::Tree& ast, TArrayView<AST::Id> moduleIds, TArrayView<String> strings)
	{
		ZoneScopedNC("DeserializeModules", 0x459bd1);
		Check(moduleIds.Size() == strings.Size());

		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			Serl::JsonFormatReader reader{strings[i]};
			if (!reader.IsValid())
			{
				continue;
			}

			auto& ct = reader.GetContext();

			ct.BeginObject();
			StringView name;
			ct.Next("name", name);
			if (!name.empty())
			{
				ast.Add<CIdentifier>(moduleIds[i], {name});
			}
		}
	}

	void DeserializeTypes(AST::Tree& ast, TArrayView<AST::Id> typeIds, TArrayView<String> strings)
	{
		ZoneScopedNC("DeserializeTypes", 0x459bd1);
		Check(typeIds.Size() == strings.Size());

		for (i32 i = 0; i < typeIds.Size(); ++i)
		{
			Serl::JsonFormatReader reader{strings[i]};
			if (!reader.IsValid())
			{
				continue;
			}

			ASTReadContext ct{reader, ast};
			ct.BeginObject();

			AST::Id entity        = typeIds[i];
			TypeCategory category = TypeCategory::None;
			ct.Next("type", category);
			Types::InitTypeFromCategory(ast, entity, category);

			ct.SerializeRoot(entity);

			// Root entity's optional name
			StringView name;
			ct.Next("name", name);
			if (!name.empty())
			{
				ast.Add<CIdentifier>(entity, {name});
			}
		}
	}
}    // namespace Rift::LoadSystem
