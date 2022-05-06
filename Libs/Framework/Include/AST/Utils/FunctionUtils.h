// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/TypeRef.h"


namespace Rift::Functions
{
	AST::Id AddInputArgument(AST::Tree& ast, AST::Id functionId, Name name = Name::None());
	AST::Id AddOutputArgument(AST::Tree& ast, AST::Id functionId, Name name = Name::None());

	AST::Id AddIf(AST::TypeRef type);
	AST::Id AddReturn(AST::TypeRef type);

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId);
	AST::Id AddCall(AST::TypeRef type, AST::Id targetFunctionId);
	AST::Id AddDeclarationReference(AST::TypeRef type, AST::Id declId);
	AST::Id AddUnaryOperator(AST::TypeRef type, UnaryOperatorType operatorType);
	AST::Id AddBinaryOperator(AST::TypeRef type, BinaryOperatorType operatorType);

	AST::Id FindFunctionByName(AST::Tree& ast, Name ownerName, Name functionName);

	using RemoveAccess = TAccess<TWrite<CChanged>, TWrite<CFileDirty>, TWrite<CStmtInput>,
	    TWrite<CStmtOutputs>, TWrite<CParent>, TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, TSpan<AST::Id> ids);

	void GetCallArgs(AST::Tree& ast, TSpan<AST::Id> callIds, TArray<AST::Id>& inputArgIds,
	    TArray<AST::Id>& outputArgIds, TArray<AST::Id>& otherIds);
}    // namespace Rift::Functions
