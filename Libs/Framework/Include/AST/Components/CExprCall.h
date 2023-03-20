// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"

#include <Pipe/Reflect/ReflectionFlags.h>
#include <Pipe/Serialize/SerializationFwd.h>


namespace rift::AST
{
	struct CExprCall : public CExpression
	{
		STRUCT(CExprCall, CExpression)

		PROP(function)
		Namespace function;
	};

	inline void Read(p::Reader& ct, CExprCall& val)
	{
		ct.Serialize(val.function);
	}
	inline void Write(p::Writer& ct, const CExprCall& val)
	{
		ct.Serialize(val.function);
	}


	// Data pointing to the id of the function from CExprCall's type and function names
	struct CExprCallId : public CExpression
	{
		STRUCT(CExprCallId, CExpression, p::Struct_NotSerialized)

		// Id pointing to the function declaration
		PROP(functionId)
		Id functionId = NoId;


		CExprCallId(Id functionId = NoId) : functionId{functionId} {}
	};
}    // namespace rift::AST
