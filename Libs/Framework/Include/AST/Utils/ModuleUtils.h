// Copyright 2015-2022 Piperift - All rights reserved

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

namespace rift::AST::Modules
{
	using namespace p::core;
	using namespace p::files;
	using namespace AST;

	static constexpr StringView moduleFile{"Module.rift"};

	bool CreateProject(Tree& ast, Path path);
	bool OpenProject(Tree& ast, Path path);
	void CloseProject(Tree& ast);

	Id GetProjectId(TAccessRef<CProject> access);

	Name GetProjectName(TAccessRef<CProject, CNamespace, CFileRef> access);
	Path GetProjectPath(TAccessRef<CFileRef, CProject> access);
	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access);

	bool HasProject(Tree& ast);

	// Resolve a module's name
	Name GetModuleName(TAccessRef<CNamespace, CFileRef> access, Id moduleId);

	// Resolve a module's name
	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);
	void Deserialize(AST::Tree& ast, AST::Id id, const String& data);
}    // namespace rift::AST::Modules
