// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::Functions
{
	AST::Id GetCompoundStmt(AST::TypeRef type, AST::Id functionId);

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId);
	AST::Id AddCall(AST::TypeRef type, AST::Id targetFunctionId);

	bool InsertStmtAfter(AST::TypeRef type, AST::Id stmt, AST::Id prevStmt);
	AST::Id FindStmtType(const AST::Tree& ast, AST::Id stmt);
}    // namespace Rift::Functions
