// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CLiteralFloat : public CLiteral
	{
		STRUCT(CLiteralFloat, CLiteral)

		PROP(value)
		float value = 0.f;
	};
}    // namespace Rift
