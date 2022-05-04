// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprOutputs : public Struct
	{
		STRUCT(CExprOutputs, Struct)

		PROP(types)
		TArray<AST::Id> types;

		PROP(names)
		TArray<Name> names;
	};

	struct CExprInvalidOutputs : public Struct
	{
		STRUCT(CExprInvalidOutputs, Struct)

		PROP(names)
		TArray<Name> names;
	};
}    // namespace Rift
