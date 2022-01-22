// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

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

	AST::Id GetProjectId(const AST::Tree& ast);
	Name GetProjectName(const AST::Tree& ast);
	Path GetProjectPath(const AST::Tree& ast);
	CModule* GetProjectModule(AST::Tree& ast);
	const CModule* GetProjectModule(const AST::Tree& ast);

	bool HasProject(const AST::Tree& ast);

	// Resolve a module's name
	// Reads: CIdentifier, CFileRef
	Name GetModuleName(const AST::Tree& ast, AST::Id moduleId);

	// Resolve a module's name
	// Reads: CFileRef
	Path GetModulePath(const AST::Tree& ast, AST::Id moduleId);
}    // namespace Rift::Modules
