// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "Assets/ModuleAsset.h"

#include <Memory/OwnPtr.h>


namespace Rift::Modules
{
	static constexpr StringView projectFile{"Project.rift"};

	void OpenProject(AST::Tree& ast, const Path& path);

	Name GetProjectName(const AST::Tree& ast);
	Path GetProjectPath(const struct CModulesUnique& modules);
	Name GetModuleName(TAssetPtr<ModuleAsset> module);
}    // namespace Rift::Modules
