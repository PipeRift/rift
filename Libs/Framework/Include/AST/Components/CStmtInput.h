// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CStmtInput : public Struct
	{
		STRUCT(CStmtInput, Struct)

		PROP(linkOutputNode)
		AST::Id linkOutputNode = AST::NoId;
	};
}    // namespace Rift
