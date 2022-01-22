// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace Rift
{
	struct CDeclRefExpr : public CExpression
	{
		STRUCT(CDeclRefExpr, CExpression)

		PROP(name)
		Name name;
	};
}    // namespace Rift
