// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Id.h"
#include "PRefl/ReflectionFlags.h"

#include <PSerl/SerializationFwd.h>


namespace rift
{
	using namespace p::core;


	struct CExprCall : public CExpression
	{
		STRUCT(CExprCall, CExpression)

		PROP(ownerName)
		Name ownerName;

		PROP(functionName)
		Name functionName;
	};

	// Data pointing to the id of the function from CExprCall's type and function names
	struct CExprCallId : public CExpression
	{
		STRUCT(CExprCallId, CExpression, p::Struct_NotSerialized)

		// Id pointing to the function declaration
		PROP(functionId)
		AST::Id functionId = AST::NoId;


		CExprCallId(AST::Id functionId = AST::NoId) : functionId{functionId} {}
	};
}    // namespace rift
