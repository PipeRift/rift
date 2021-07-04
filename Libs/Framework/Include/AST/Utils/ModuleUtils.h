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

	void OpenProject(AST::Tree& ast, const Path& path);
	void CloseProject(AST::Tree& ast);

	CModule* GetProjectModule(AST::Tree& ast);
	const CModule* GetProjectModule(const AST::Tree& ast);

	Name GetProjectName(const AST::Tree& ast);
	Path GetProjectPath(const AST::Tree& ast);

	Name GetModuleName(const CModule& module);
	Path GetModulePath(const CModule& module);
}    // namespace Rift::Modules
