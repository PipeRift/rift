// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Tree.h"

#include <PipeArrays.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct Tree;
}

namespace rift::ast::LoadSystem
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
	void ScanSubmodules(Tree& ast, p::TArray<p::String>& paths);
	/**
	 * @param paths of all currently unloaded types
	 */
	void ScanTypes(Tree& ast, p::TArray<ModuleTypePaths>& pathsByModule);

	void CreateModulesFromPaths(Tree& ast, p::TArray<p::String>& paths, p::TArray<Id>& ids);
	void CreateTypesFromPaths(
	    Tree& ast, p::TView<ModuleTypePaths> pathsByModule, p::TArray<Id>& ids);

	void LoadFileStrings(
	    p::TAccessRef<CFileRef> access, p::TView<Id> nodes, p::TArray<p::String>& strings);

	void DeserializeModules(Tree& ast, p::TView<Id> moduleIds, p::TView<p::String> strings);
	void DeserializeTypes(Tree& ast, p::TView<Id> typeIds, p::TView<p::String> strings);

}    // namespace rift::ast::LoadSystem
