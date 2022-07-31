// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Reflect/ReflectionFlags.h>
#include <Pipe/Serialize/SerializationFwd.h>


namespace rift
{
	using namespace p::core;


	struct CExprCall : public CExpression
	{
		STRUCT(CExprCall, CExpression)

		PROP(function)
		AST::Namespace function;
	};

	static void Read(Reader& ct, CExprCall& val)
	{
		ct.Serialize(val.function);
	}
	static void Write(Writer& ct, const CExprCall& val)
	{
		ct.Serialize(val.function);
	}


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
