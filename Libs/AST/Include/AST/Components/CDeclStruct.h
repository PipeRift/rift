// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclRecord.h"


namespace rift::AST
{
	struct CDeclStruct : public CDeclRecord
	{
		P_STRUCT(CDeclStruct, CDeclRecord, )
	};
}    // namespace rift::AST
