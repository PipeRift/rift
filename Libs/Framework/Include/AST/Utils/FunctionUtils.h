// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::AST::Functions
{
	Id AddInputArgument(Tree& ast, Id functionId);
	Id AddOutputArgument(Tree& ast, Id functionId);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddCall(TypeRef type, Id targetFunctionId);

	Id FindFunctionByName(const Tree& ast, Name typeName, Name functionName);
}    // namespace Rift::AST::Functions
