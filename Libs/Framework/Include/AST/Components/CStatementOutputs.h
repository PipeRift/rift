// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CStatementOutputs : public Struct
	{
		STRUCT(CStatementOutputs, Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(linkPins)
		TArray<AST::Id> linkPins;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;
	};
}    // namespace Rift
