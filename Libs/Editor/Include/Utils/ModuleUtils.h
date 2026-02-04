// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "AST/Tree.h"
#include "Components/CModuleEditor.h"

#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>


namespace rift::editor
{
	void OpenModuleEditor(p::TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id);
	void CloseModuleEditor(p::TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id);
	bool IsEditingModule(p::TAccessRef<CModuleEditor> access, ast::Id id);
}    // namespace rift::editor
