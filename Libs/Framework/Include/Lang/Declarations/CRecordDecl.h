// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/CDeclaration.h"

#include <Lang/AST.h>


namespace Rift
{
	struct CRecordDecl : public CDeclaration
	{
		STRUCT(CRecordDecl, CDeclaration)

		// AST nodes representing data fields of this record
		TArray<AST::Id> data;
	};
}    // namespace Rift
