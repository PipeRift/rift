// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Pipe/Core/String.h>


namespace rift::AST
{
	struct CLiteralString : public CLiteral
	{
		STRUCT(CLiteralString, CLiteral)

		PROP(value)
		p::String value;
	};
}    // namespace rift::AST
