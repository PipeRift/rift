// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CRecordDecl.h"

#include <AST/Tree.h>


namespace Rift
{
	struct CClassDecl : public CRecordDecl
	{
		STRUCT(CClassDecl, CRecordDecl)
	};
}    // namespace Rift
