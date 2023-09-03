// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace rift::AST
{
	/** Represents a return expression of a function
	 * Return arguments are dynamically populated depending on the function this expression is
	 * connected to.
	 */
	struct CStmtReturn : public CExpression
	{
		P_STRUCT(CStmtReturn, CExpression)
	};
}    // namespace rift::AST
