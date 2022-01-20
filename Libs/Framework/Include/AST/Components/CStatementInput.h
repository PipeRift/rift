// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CStatementInput : public Struct
	{
		STRUCT(CStatementInput, Struct)

		PROP(linkOutputNode)
		AST::Id linkOutputNode = AST::NoId;
	};
}    // namespace Rift
