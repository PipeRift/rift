// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Containers/Array.h>
#include <Containers/ArrayView.h>


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::LoadSystem
{
	struct ModuleTypePaths
	{
		AST::Id moduleId;
		TArray<Path> paths;    // Paths of module types
		TArray<Name> pathNames;
	};

	void Init(AST::Tree& ast);
	void Run(AST::Tree& ast);

	void LoadSubmodules(AST::Tree& ast);
	void LoadTypes(AST::Tree& ast);

	/**
	 * @param paths of all currently unloaded modules
	 */
	void ScanSubmodules(AST::Tree& ast, TArray<Path>& paths);
	/**
	 * @param paths of all currently unloaded types
	 */
	void ScanTypes(AST::Tree& ast, TArray<ModuleTypePaths>& pathsByModule);

	void CreateModulesFromPaths(AST::Tree& ast, TArray<Path>& paths, TArray<AST::Id>& ids);
	void CreateTypesFromPaths(
	    AST::Tree& ast, TArrayView<ModuleTypePaths> pathsByModule, TArray<AST::Id>& ids);

	void LoadFileStrings(AST::Tree& ast, TArrayView<AST::Id> nodes, TArray<String>& strings);

	void DeserializeModules(
	    AST::Tree& ast, TArrayView<AST::Id> moduleIds, TArrayView<String> strings);
	void DeserializeTypes(AST::Tree& ast, TArrayView<AST::Id> typeIds, TArrayView<String> strings);

}    // namespace Rift::LoadSystem
