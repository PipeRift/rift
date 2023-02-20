// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace rift::AST
{
	struct CExprDeclRef : public CExpression
	{
		STRUCT(CExprDeclRef, CExpression)

		PROP(ownerName)
		p::Tag ownerName;

		PROP(name)
		p::Tag name;
	};

	struct CExprDeclRefId : public CExpression
	{
		STRUCT(CExprDeclRefId, CExpression)

		PROP(declarationId)
		Id declarationId;
	};
}    // namespace rift::AST
