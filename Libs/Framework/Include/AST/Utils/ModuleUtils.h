// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CProject.h"
#include "AST/Tree.h"

#include <ECS/Filtering.h>
#include <Memory/OwnPtr.h>


namespace rift
{
	struct CModule;
}

namespace rift::Modules
{
	using namespace pipe::core;
	using namespace pipe::Files;
	using namespace AST;

	static constexpr StringView moduleFile{"Module.rift"};

	bool CreateProject(Tree& ast, Path path);
	bool OpenProject(Tree& ast, Path path);
	void CloseProject(Tree& ast);

	Id GetProjectId(TAccessRef<CProject> access);

	Name GetProjectName(TAccessRef<CProject, CIdentifier, CFileRef> access);
	Path GetProjectPath(TAccessRef<CFileRef, CProject> access);
	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access);

	bool HasProject(Tree& ast);

	// Resolve a module's name
	Name GetModuleName(TAccessRef<CIdentifier, CFileRef> access, Id moduleId);

	// Resolve a module's name
	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);
	void Deserialize(AST::Tree& ast, AST::Id id, const String& data);
}    // namespace rift::Modules
