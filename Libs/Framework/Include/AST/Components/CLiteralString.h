// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CLiteralString : public CLiteral
	{
		STRUCT(CLiteralString, CLiteral)

		PROP(value)
		String value;
	};
}    // namespace Rift
