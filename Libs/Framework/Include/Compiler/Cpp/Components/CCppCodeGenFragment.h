// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CCppCodeGenFragment : public Struct
	{
		STRUCT(CCppCodeGenFragment, Struct)

		String code;
	};
}    // namespace Rift
