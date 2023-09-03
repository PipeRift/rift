// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace rift::AST
{
	struct CExprDeclRef : public CExpression
	{
		P_STRUCT(CExprDeclRef, CExpression)

		P_PROP(ownerName)
		p::Tag ownerName;

		P_PROP(name)
		p::Tag name;
	};

	struct CExprDeclRefId : public CExpression
	{
		P_STRUCT(CExprDeclRefId, CExpression)

		P_PROP(declarationId)
		Id declarationId;
	};
}    // namespace rift::AST
