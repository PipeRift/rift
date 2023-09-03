// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Pipe/Core/String.h>


namespace rift::AST
{
	struct CLiteralString : public CLiteral
	{
		P_STRUCT(CLiteralString, CLiteral)

		P_PROP(value)
		p::String value;
	};
}    // namespace rift::AST
