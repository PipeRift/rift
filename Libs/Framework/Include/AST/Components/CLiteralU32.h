// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CLiteralU32 : public CLiteral
	{
		STRUCT(CLiteralU32, CLiteral)

		PROP(value)
		u32 value = 0;
	};
}    // namespace Rift
