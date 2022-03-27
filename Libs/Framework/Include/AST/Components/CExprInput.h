// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprInput : public Struct
	{
		STRUCT(CExprInput, Struct)

		PROP(linkOutputPin)
		AST::Id linkOutputPin = AST::NoId;
	};
}    // namespace Rift
