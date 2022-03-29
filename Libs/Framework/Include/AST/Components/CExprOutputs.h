// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprOutputs : public Struct
	{
		STRUCT(CExprOutputs, Struct)

		PROP(linkInputPins)
		TArray<AST::Id> linkInputPins;
	};
}    // namespace Rift
