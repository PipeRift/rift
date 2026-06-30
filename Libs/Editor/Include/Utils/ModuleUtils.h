// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "AST/Tree.h"
#include "Components/CModuleEditor.h"

#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>


namespace rift::editor
{
	void OpenModuleEditor(p::TIdScopeRef<Writes<CModuleEditor>, ast::CModule> scope, ast::Id id);
	void CloseModuleEditor(p::TIdScopeRef<Writes<CModuleEditor>, ast::CModule> scope, ast::Id id);
	bool IsEditingModule(p::TIdScopeRef<CModuleEditor> scope, ast::Id id);
}    // namespace rift::editor
