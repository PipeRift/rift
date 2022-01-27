// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Types.h"
#include "Reflection/ReflectionFlags.h"

#include <Serialization/ContextsFwd.h>


namespace Rift
{
	struct CCallExpr : public CExpression
	{
		STRUCT(CCallExpr, CExpression)

		PROP(ownerName)
		Name ownerName;

		PROP(functionName)
		Name functionName;
	};

	// Data pointing to the id of the function from CCallExpr's type and function names
	struct CCallExprId : public CExpression
	{
		STRUCT(CCallExprId, CExpression, Struct_NotSerialized)

		// Id pointing to the function declaration
		PROP(functionId)
		AST::Id functionId = AST::NoId;


		CCallExprId(AST::Id functionId = AST::NoId) : functionId{functionId} {}
	};
}    // namespace Rift
