// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclRecord.h"


namespace rift
{
	struct CDeclStruct : public CDeclRecord
	{
		STRUCT(CDeclStruct, CDeclRecord)
	};
}    // namespace rift