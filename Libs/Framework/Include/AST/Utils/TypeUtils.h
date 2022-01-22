// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category);
	TypeCategory GetCategory(AST::Tree& ast, AST::Id id);

	AST::Id CreateType(AST::Tree& ast, TypeCategory type, Name name = Name::None());

	AST::Id AddVariable(AST::TypeRef type, Name name);
	AST::Id AddFunction(AST::TypeRef type, Name name);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);
	void Deserialize(AST::Tree& ast, AST::Id id, const String& data);

	bool IsClass(const AST::Tree& ast, AST::Id typeId);
	bool IsStruct(const AST::Tree& ast, AST::Id typeId);
	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId);
	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId);
	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId);
}    // namespace Rift::Types
