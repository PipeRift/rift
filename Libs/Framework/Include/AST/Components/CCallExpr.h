// Copyright 2015-2020 Piperift - All rights reserved
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

		// Id pointing to the function declaration. Resolved after loading
		PROP(functionId, Prop_Transient)
		AST::Id functionId = AST::NoId;

		PROP(typeName)
		Name typeName;

		PROP(functionName)
		Name functionName;
	};
}    // namespace Rift
