// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <AST/Components/Declarations.h>
#include <PipeReflect.h>


namespace rift
{
	struct CDeclCStruct : public ast::CDeclRecord
	{
		using Super = CDeclRecord;
		P_STRUCT(CDeclCStruct)
	};

	struct CDeclCStatic : public ast::CDeclRecord
	{
		using Super = CDeclRecord;
		P_STRUCT(CDeclCStatic)
	};
}    // namespace rift
