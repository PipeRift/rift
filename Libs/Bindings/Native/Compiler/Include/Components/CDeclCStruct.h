// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Components/CDeclRecord.h>


namespace rift
{
	struct CDeclCStruct : public AST::CDeclRecord
	{
		STRUCT(CDeclCStruct, CDeclRecord)
	};
}    // namespace rift
