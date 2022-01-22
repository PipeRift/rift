// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"

#include <AST/Tree.h>


namespace Rift
{
	struct CRecordDecl : public CDeclaration
	{
		STRUCT(CRecordDecl, CDeclaration)
	};
}    // namespace Rift
