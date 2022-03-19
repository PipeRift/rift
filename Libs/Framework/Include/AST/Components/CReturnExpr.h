// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace Rift
{
	struct CReturnExpr : public CExpression
	{
		STRUCT(CReturnExpr, CExpression)

		PROP(id)
		AST::Id id = AST::NoId;


		CReturnExpr() = default;
		CReturnExpr(AST::Id id) : id{id} {}
	};
}    // namespace Rift
