// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "Assets/ModuleAsset.h"

#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CModule;
}

namespace Rift::Modules
{
	static constexpr StringView projectFile{"Project.rift"};

	AST::Tree OpenProject(const Path& path);
	void CloseProject(AST::Tree& ast);

	AST::Id GetProjectModule(const AST::Tree& ast);

	Name GetProjectName(const AST::Tree& ast);
	const Path& GetProjectPath(const AST::Tree& ast);

	bool HasProject(const AST::Tree& ast);

	// Resolve a module's name
	// Reads: CIdentifier, CModule
	Name GetModuleName(const AST::Tree& ast, AST::Id moduleId);
}    // namespace Rift::Modules
