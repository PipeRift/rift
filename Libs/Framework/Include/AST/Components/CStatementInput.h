// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CStatementInput : public Struct
	{
		STRUCT(CStatementInput, Struct)

		PROP(edgeOutputNode)
		AST::Id edgeOutputNode = AST::NoId;
		PROP(edgeOutputPin)
		AST::Id edgeOutputPin = AST::NoId;
	};
}    // namespace Rift
