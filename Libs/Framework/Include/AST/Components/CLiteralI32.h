// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CLiteralI32 : public CLiteral
	{
		STRUCT(CLiteralI32, CLiteral)

		PROP(value)
		i32 value = 0;
	};
}    // namespace Rift
