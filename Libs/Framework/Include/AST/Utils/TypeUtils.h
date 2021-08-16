// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <AST/Components/CType.h>


namespace Rift::Types
{
	void InitFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category);
	TypeCategory GetCategory(AST::Tree& ast, AST::Id id);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);

	// TODO: Move from load system
	// void Deserialize(AST::Tree& ast, AST::Id id, String& data);
}    // namespace Rift::Types
