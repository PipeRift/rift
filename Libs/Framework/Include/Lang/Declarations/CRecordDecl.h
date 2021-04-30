// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/CDeclaration.h"

#include <ECS.h>


namespace Rift
{
	struct CRecordDecl : public CDeclaration
	{
		STRUCT(CRecordDecl, CDeclaration)

		// AST nodes representing data fields of this record
		TArray<ECS::EntityId> data;
	};
}    // namespace Rift
