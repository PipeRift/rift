// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"


namespace rift
{
	using namespace p::core;


	struct CExprDeclRef : public CExpression
	{
		STRUCT(CExprDeclRef, CExpression)

		PROP(ownerName)
		Name ownerName;

		PROP(name)
		Name name;
	};

	struct CExprDeclRefId : public CExpression
	{
		STRUCT(CExprDeclRefId, CExpression)

		PROP(declarationId)
		AST::Id declarationId;
	};
}    // namespace rift
