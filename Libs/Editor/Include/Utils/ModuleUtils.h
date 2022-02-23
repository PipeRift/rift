// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "Components/CModuleEditor.h"

#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>


namespace Rift::Modules
{
	void OpenEditor(TAccessRef<TWrite<CModuleEditor>, CModule> access, AST::Id id);
	void CloseEditor(TAccessRef<TWrite<CModuleEditor>, CModule> access, AST::Id id);
	bool IsEditorOpen(TAccessRef<CModuleEditor> access, AST::Id id);
}    // namespace Rift::Modules
