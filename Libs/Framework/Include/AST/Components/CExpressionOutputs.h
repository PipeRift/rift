// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CExpressionOutputs : public Struct
	{
		STRUCT(CExpressionOutputs, Struct)

		PROP(linkInputPins)
		TArray<AST::Id> linkInputPins;
	};
}    // namespace Rift
