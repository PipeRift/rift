// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace rift::AST
{
	struct CLiteralBool : public CLiteral
	{
		P_STRUCT(CLiteralBool, CLiteral)

		P_PROP(value)
		bool value = false;
	};
}    // namespace rift::AST
