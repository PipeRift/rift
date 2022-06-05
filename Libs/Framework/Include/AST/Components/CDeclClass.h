// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclRecord.h"

#include <AST/Tree.h>


namespace rift
{
	struct CDeclClass : public CDeclRecord
	{
		STRUCT(CDeclClass, CDeclRecord)
	};
}    // namespace rift
