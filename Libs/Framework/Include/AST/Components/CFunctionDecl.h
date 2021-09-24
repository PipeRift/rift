// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"

#include <AST/Tree.h>


namespace Rift
{
	struct CFunctionDecl : public CDeclaration
	{
		STRUCT(CFunctionDecl, CDeclaration)
	};
}    // namespace Rift
