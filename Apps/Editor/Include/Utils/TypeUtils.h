// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <AST/Utils/TypeUtils.h>


namespace Rift::Types
{
	void OpenType(AST::Tree& ast, AST::Id typeId);
	void CloseType(AST::Tree& ast, AST::Id typeId);
	bool IsTypeOpen(AST::Tree& ast, AST::Id typeId);

	/**
	 * Called after a change has been done to a type.
	 * Stores an image of the current state of the type for Undos and Redos.
	 * TODO: Not implemented
	 */
	void Changed(AST::Id typeId, StringView name);

	void OpenFunction(AST::Tree& ast, AST::Id functionId);
	void CloseFunction(AST::Tree& ast, AST::Id functionId);
}    // namespace Rift::Types
