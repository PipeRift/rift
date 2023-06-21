// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Tree.h"

#include <Pipe/Core/Array.h>
#include <Pipe/Core/Span.h>
#include <Pipe/PipeECS.h>


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
	void CreateTypesFromPaths(Tree& ast, TSpan<ModuleTypePaths> pathsByModule, TArray<Id>& ids);

	void LoadFileStrings(TAccessRef<CFileRef> access, TSpan<Id> nodes, TArray<String>& strings);

	void DeserializeModules(Tree& ast, TSpan<Id> moduleIds, TSpan<String> strings);
	void DeserializeTypes(Tree& ast, TSpan<Id> typeIds, TSpan<String> strings);

}    // namespace rift::AST::LoadSystem
