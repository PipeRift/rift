// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace Rift
{
	struct CDeclRefExpr : public CExpression
	{
		STRUCT(CDeclRefExpr, CExpression)

		PROP(ownerName)
		Name ownerName;

		PROP(name)
		Name name;
	};

	struct CDeclRefExprId : public CExpression
	{
		STRUCT(CDeclRefExprId, CExpression)

		PROP(declarationId)
		AST::Id declarationId;
	};
}    // namespace Rift
