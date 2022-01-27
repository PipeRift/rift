// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::AST::Functions
{
	Id AddInputArgument(Tree& ast, Id functionId);
	Id AddOutputArgument(Tree& ast, Id functionId);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddCall(TypeRef type, Id targetFunctionId);
	Id AddDeclarationReference(TypeRef type, Id declId);

	Id FindFunctionByName(const Tree& ast, Name ownerName, Name functionName);

	void GetCallArgs(const Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
	    TArray<Id>& outputArgIds, TArray<Id>& otherIds);
}    // namespace Rift::AST::Functions
