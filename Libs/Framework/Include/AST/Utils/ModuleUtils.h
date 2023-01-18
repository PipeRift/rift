// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CProject.h"
#include "AST/Tree.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/Memory/OwnPtr.h>


namespace rift::AST
{
	struct CModule;
}

namespace rift::AST
{
	using namespace p::core;
	using namespace p::files;
	using namespace AST;

	bool CreateProject(Tree& ast, Path path);
	bool OpenProject(Tree& ast, Path path);
	void CloseProject(Tree& ast);

	Id CreateModule(Tree& ast, Path path);

	Id GetProjectId(TAccessRef<CProject> access);

	Name GetProjectName(TAccessRef<CProject, CNamespace, CFileRef> access);
	Path GetProjectPath(TAccessRef<CFileRef, CProject> access);
	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access);

	bool HasProject(Tree& ast);

	// Resolve a module's name
	Name GetModuleName(TAccessRef<CNamespace, CFileRef> access, Id moduleId);

	// Resolve a module's name
	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId);

	void SerializeModule(AST::Tree& ast, AST::Id id, String& data);
	void DeserializeModule(AST::Tree& ast, AST::Id id, const String& data);
}    // namespace rift::AST
