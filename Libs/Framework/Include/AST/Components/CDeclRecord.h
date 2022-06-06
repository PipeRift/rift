// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"

#include <AST/Tree.h>


namespace rift
{
	struct CDeclRecord : public CDeclaration
	{
		STRUCT(CDeclRecord, CDeclaration)
	};
}    // namespace rift
