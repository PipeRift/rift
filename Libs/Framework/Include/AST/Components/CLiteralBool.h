// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace rift::AST
{
	struct CLiteralBool : public CLiteral
	{
		STRUCT(CLiteralBool, CLiteral)

		PROP(value)
		bool value = false;
	};
}    // namespace rift::AST
