// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"

#include <AST/Tree.h>


namespace rift
{
	struct CDeclFunction : public CDeclaration
	{
		STRUCT(CDeclFunction, CDeclaration)
	};
}    // namespace rift