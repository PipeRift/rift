// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExpression.h"
#include "AST/Types.h"
#include "Reflection/ReflectionFlags.h"

#include <Serialization/ContextsFwd.h>


namespace Rift
{
	struct CInvalid : public Struct
	{
		STRUCT(CInvalid, Struct)
	};
}    // namespace Rift
