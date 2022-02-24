// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "Components/CTypeEditor.h"

#include <AST/Components/CType.h>
#include <AST/Utils/TypeUtils.h>


namespace Rift::Types
{
	void OpenEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id);
	void CloseEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id);
	bool IsEditorOpen(TAccessRef<CTypeEditor> access, AST::Id id);
}    // namespace Rift::Types
