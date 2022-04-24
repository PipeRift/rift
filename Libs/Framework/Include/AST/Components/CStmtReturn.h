// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace Rift
{
	/** Represents a return expression of a function
	 * Return arguments are dynamically populated depending on the function this expression is
	 * connected to.
	 */
	struct CStmtReturn : public CExpression
	{
		STRUCT(CStmtReturn, CExpression)
	};
}    // namespace Rift
