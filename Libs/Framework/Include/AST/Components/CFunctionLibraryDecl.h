// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"

#include <AST/Tree.h>


namespace Rift
{
	struct CFunctionLibraryDecl : public CDeclaration
	{
		STRUCT(CFunctionLibraryDecl, CDeclaration)
	};
}    // namespace Rift
