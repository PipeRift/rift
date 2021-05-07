// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declarations/CRecordDecl.h"

#include <Lang/AST.h>


namespace Rift
{
	struct CClassDecl : public CRecordDecl
	{
		STRUCT(CClassDecl, CRecordDecl)
	};
}    // namespace Rift
