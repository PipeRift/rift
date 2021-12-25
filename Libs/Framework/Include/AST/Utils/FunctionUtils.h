// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::AST::Functions
{
	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddCall(TypeRef type, Id targetFunctionId);

	Id FindFunctionByName(const Tree& ast, Name typeName, Name functionName);
}    // namespace Rift::AST::Functions
