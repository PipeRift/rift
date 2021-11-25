// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/TypeRef.h"

#include <AST/Components/CType.h>


namespace Rift::Functions
{
	AST::Id GetCompoundStmt(AST::TypeRef type, AST::Id functionId);

	/**
	 * @brief Connects an statement to a previous statement or statement compound (making it the
	 * first )
	 * @param ast
	 * @param ownerTypeId
	 * @param statementId
	 * @param targetId
	 * @return true if connection succeeded
	 */
	bool ExtractStatements(AST::TypeRef type, AST::Id statementId, AST::Id targetId) {}
	bool InsertStatement(AST::TypeRef type, AST::Id statement, TSpan<AST::Id> statements) {}
}    // namespace Rift::Functions
