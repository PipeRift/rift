// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CStatementOutputs : public Struct
	{
		STRUCT(CStatementOutputs, Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(edgePins)
		TArray<AST::Id> edgePins;
		PROP(edgeInputNodes)
		TArray<AST::Id> edgeInputNodes;
	};
}    // namespace Rift
