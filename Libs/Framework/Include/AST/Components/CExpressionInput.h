// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExpressionInput : public Struct
	{
		STRUCT(CExpressionInput, Struct)

		PROP(linkOutputPin)
		AST::Id linkOutputPin = AST::NoId;
	};
}    // namespace Rift
