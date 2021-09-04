// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace Rift
{
	struct CCallExpr : public CExpression
	{
		STRUCT(CCallExpr, CExpression)

		PROP(type)
		Name type;

		PROP(name)
		Name name;
	};
}    // namespace Rift
