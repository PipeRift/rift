// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Types.h"


namespace Rift
{
	struct CCallExpr : public CExpression
	{
		STRUCT(CCallExpr, CExpression)

		PROP(functionId, Prop_Transient)
		AST::Id functionId = AST::NoId;
	};
}    // namespace Rift
