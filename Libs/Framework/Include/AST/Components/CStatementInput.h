// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CStatementInput : public Struct
	{
		STRUCT(CStatementInput, Struct)

		PROP(output)
		AST::Id output;
	};
}    // namespace Rift
