// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CStmtOutputs : public Struct
	{
		STRUCT(CStmtOutputs, Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(linkPins)
		TArray<AST::Id> linkPins;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;
	};
}    // namespace Rift