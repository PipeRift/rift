// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Tree.h"

#include <Core/Array.h>
#include <Core/Span.h>
#include <ECS/Filtering.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::LoadSystem
{
	struct ModuleTypePaths
	{
		AST::Id moduleId;
		TArray<Path> paths;    // p::Paths of module types
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
	    AST::Tree& ast, TSpan<ModuleTypePaths> pathsByModule, TArray<AST::Id>& ids);

	void LoadFileStrings(
	    TAccessRef<CFileRef> access, TSpan<AST::Id> nodes, TArray<String>& strings);

	void DeserializeModules(AST::Tree& ast, TSpan<AST::Id> moduleIds, TSpan<String> strings);
	void DeserializeTypes(AST::Tree& ast, TSpan<AST::Id> typeIds, TSpan<String> strings);

}    // namespace rift::LoadSystem
