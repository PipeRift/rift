// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Tree.h"


namespace rift::ast::NameSystem
{
	/** Assigns a unique C-safe name to every entity that needs one, stored as a
	 * CGeneratedName component. Names avoid conflicts with their enclosing scopes.
	 */
	void AssignGeneratedNames(Tree& ast);
}    // namespace rift::ast::NameSystem
