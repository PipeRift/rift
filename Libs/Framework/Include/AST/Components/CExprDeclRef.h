// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace rift::AST
{
	struct CExprDeclRef : public CExpression
	{
		STRUCT(CExprDeclRef, CExpression)

		PROP(ownerName)
		p::Name ownerName;

		PROP(name)
		p::Name name;
	};

	struct CExprDeclRefId : public CExpression
	{
		STRUCT(CExprDeclRefId, CExpression)

		PROP(declarationId)
		Id declarationId;
	};
}    // namespace rift::AST
