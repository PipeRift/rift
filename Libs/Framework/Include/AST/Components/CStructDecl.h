// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CRecordDecl.h"


namespace Rift
{
	struct CStructDecl : public CRecordDecl
	{
		STRUCT(CStructDecl, CRecordDecl)
	};
}    // namespace Rift
