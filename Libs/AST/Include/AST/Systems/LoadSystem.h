// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Tree.h"

#include <PipeArrays.h>
#include <PipeECS.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::AST::LoadSystem
{
	struct ModuleTypePaths
	{
		Id moduleId;
		p::TArray<p::String> paths;    // p::Paths of module types
	};

	void Init(Tree& ast);
	void Run(Tree& ast);

	void LoadSubmodules(Tree& ast);
	void LoadTypes(Tree& ast);

	/**
	 * @param paths of all currently unloaded modules
	 */
	void ScanSubmodules(Tree& ast, TArray<String>& paths);
	/**
	 * @param paths of all currently unloaded types
	 */
	void ScanTypes(Tree& ast, TArray<ModuleTypePaths>& pathsByModule);

	void CreateModulesFromPaths(Tree& ast, TArray<String>& paths, TArray<Id>& ids);
	void CreateTypesFromPaths(Tree& ast, TView<ModuleTypePaths> pathsByModule, TArray<Id>& ids);

	void LoadFileStrings(TAccessRef<CFileRef> access, TView<Id> nodes, TArray<String>& strings);

	void DeserializeModules(Tree& ast, TView<Id> moduleIds, TView<String> strings);
	void DeserializeTypes(Tree& ast, TView<Id> typeIds, TView<String> strings);

}    // namespace rift::AST::LoadSystem
