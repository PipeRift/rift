// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declarations/FunctionDecl.h"
#include "Lang/Declarations/RecordDecl.h"

namespace Rift
{
	struct ClassDecl : public RecordDecl
	{
		STRUCT(ClassDecl, RecordDecl)

		TArray<TOwnPtr<FunctionDecl>> functions;
	};
}    // namespace Rift
