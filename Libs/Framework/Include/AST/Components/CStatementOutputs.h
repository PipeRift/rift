// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CStatementOutputs : public Struct
	{
		STRUCT(CStatementOutputs, Struct)

		PROP(edgeInputNodes)
		TArray<AST::Id> edgeInputNodes;
	};
}    // namespace Rift
