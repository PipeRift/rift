// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "Components/CModuleEditor.h"

#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>


namespace rift::Editor
{
	void OpenModuleEditor(TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id);
	void CloseModuleEditor(TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id);
	bool IsEditingModule(TAccessRef<CModuleEditor> access, ast::Id id);
}    // namespace rift::Editor
