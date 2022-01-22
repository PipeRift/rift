// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CStringLiteral : public CLiteral
	{
		STRUCT(CStringLiteral, CLiteral)

		PROP(value)
		String value;
	};
}    // namespace Rift
