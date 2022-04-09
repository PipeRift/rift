// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CType.h"
#include "AST/TypeRef.h"


namespace Rift::AST::Functions
{
	Id AddInputArgument(Tree& ast, Id functionId);
	Id AddOutputArgument(Tree& ast, Id functionId);

	Id AddIf(TypeRef type);
	Id AddReturn(TypeRef type);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddCall(TypeRef type, Id targetFunctionId);
	Id AddDeclarationReference(TypeRef type, Id declId);
	Id AddUnaryOperator(TypeRef type, UnaryOperatorType operatorType);
	Id AddBinaryOperator(TypeRef type, BinaryOperatorType operatorType);

	Id FindFunctionByName(Tree& ast, Name ownerName, Name functionName);


	void GetCallArgs(Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
	    TArray<Id>& outputArgIds, TArray<Id>& otherIds);
}    // namespace Rift::AST::Functions
