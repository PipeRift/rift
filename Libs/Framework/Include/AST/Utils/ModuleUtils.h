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
	static constexpr StringView moduleFile{"Module.rift"};

	bool OpenProject(AST::Tree& ast, const Path& path);
	void CloseProject(AST::Tree& ast);

	AST::Id GetProjectId(AST::TAccessRef<const CProject> access);

	using GetProjectNameAccess = AST::TAccessRef<const CProject, const CIdentifier, const CFileRef>;
	Name GetProjectName(GetProjectNameAccess access);
	Path GetProjectPath(AST::TAccessRef<const CFileRef, const CProject> access);
	CModule* GetProjectModule(AST::TAccessRef<const CProject, CModule> access);

	bool HasProject(AST::Tree& ast);

	// Resolve a module's name
	// Reads: CIdentifier, CFileRef
	using GetModuleNameAccess = AST::TAccessRef<const CIdentifier, const CFileRef>;
	Name GetModuleName(GetModuleNameAccess access, AST::Id moduleId);

	// Resolve a module's name
	// Reads: CFileRef
	Path GetModulePath(AST::TAccessRef<const CFileRef> access, AST::Id moduleId);
}    // namespace Rift::Modules
