// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprInput : public Struct
	{
		STRUCT(CExprInput, Struct)

		PROP(linkedOutput)
		AST::Id linkedOutput = AST::NoId;

		PROP(type)
		AST::Id type = AST::NoId;

		PROP(name)
		Name name;
	};

	struct CExprInputs : public Struct
	{
		STRUCT(CExprInputs, Struct)

		PROP(linkedOutputs)
		TArray<AST::Id> linkedOutputs;

		PROP(types)
		TArray<AST::Id> types;

		PROP(names)
		TArray<Name> names;
	};
}    // namespace Rift
