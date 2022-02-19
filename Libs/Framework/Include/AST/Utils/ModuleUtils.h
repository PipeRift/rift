// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CProject.h"
#include "AST/Filtering.h"
#include "AST/Tree.h"

#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CModule;
}

namespace Rift::Modules
{
	using namespace AST;

	static constexpr StringView moduleFile{"Module.rift"};

	bool OpenProject(Tree& ast, const Path& path);
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
}    // namespace Rift::Modules
