// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclRecord.h"


namespace rift::AST
{
	struct CDeclStruct : public CDeclRecord
	{
		STRUCT(CDeclStruct, CDeclRecord, )
	};
}    // namespace rift::AST
