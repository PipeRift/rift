// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"


namespace Rift
{
	struct CBoolLiteral : public CLiteral
	{
		STRUCT(CBoolLiteral, CLiteral)

		PROP(value)
		bool value = false;
	};
}    // namespace Rift
