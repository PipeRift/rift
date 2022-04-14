// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInput.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
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

	using RemoveAccess =
	    TAccess<TWrite<CChanged>, TWrite<CFileDirty>, TWrite<CExprInput>, TWrite<CExprOutputs>,
	        TWrite<CStmtInput>, TWrite<CStmtOutputs>, TWrite<CParent>, TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, TSpan<Id> ids);

	void GetCallArgs(Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
	    TArray<Id>& outputArgIds, TArray<Id>& otherIds);
}    // namespace Rift::AST::Functions
