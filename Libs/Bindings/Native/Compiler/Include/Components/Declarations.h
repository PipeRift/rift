// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Components/Declarations.h>


namespace rift
{
	struct CDeclCStruct : public AST::CDeclRecord
	{
		P_STRUCT(CDeclCStruct, CDeclRecord)
	};

	struct CDeclCStatic : public AST::CDeclRecord
	{
		P_STRUCT(CDeclCStatic, CDeclRecord)
	};
}    // namespace rift
