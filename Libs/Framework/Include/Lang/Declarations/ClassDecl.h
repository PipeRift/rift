// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declarations/RecordDecl.h"

#include <ECS.h>


namespace Rift
{
	struct CClassDecl : public CRecordDecl
	{
		STRUCT(CClassDecl, CRecordDecl)

		// List of AST function declarations
		TArray<ECS::EntityId> functions;
	};
}    // namespace Rift
