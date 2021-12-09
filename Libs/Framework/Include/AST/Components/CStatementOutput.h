// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CStatementOutput : public Struct
	{
		STRUCT(CStatementOutput, Struct)

		PROP(input)
		AST::Id input;
	};
}    // namespace Rift
