// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclRecord.h"


namespace rift::AST
{
	struct CDeclClass : public CDeclRecord
	{
		STRUCT(CDeclClass, CDeclRecord)
	};
}    // namespace rift::AST
