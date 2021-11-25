// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category);
	TypeCategory GetCategory(AST::Tree& ast, AST::Id id);

	AST::Id CreateClass(AST::Tree& ast, Name name);
	AST::Id CreateStruct(AST::Tree& ast, Name name);

	AST::Id AddVariable(AST::TypeRef type, Name name);
	AST::Id AddFunction(AST::TypeRef type, Name name);

	AST::Id CreateLiteral(AST::Tree& ast, AST::Id typeId, AST::Id parentId);
	AST::Id CreateCall(AST::Tree& ast, AST::Id functionId, AST::Id parentId);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);

	// TODO: Move from load system
	// void Deserialize(AST::Tree& ast, AST::Id id, String& data);
}    // namespace Rift::Types
