// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	struct SStatementGraph : public Struct
	{
		STRUCT(SStatementGraph, Struct)

		TMap<AST::Id, AST::Id> inputToOutputs;
		TMap<AST::Id, AST::Id> inputToPins;


		AST::Id GetOutputPin(AST::Id edge) const
		{
			// inputToPins always has a value, even if it is NoId
			return *inputToPins.FindIt(edge);
		}
	};
}    // namespace Rift
