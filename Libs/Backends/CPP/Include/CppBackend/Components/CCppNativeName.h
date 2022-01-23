// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CCppNativeName : public Struct
	{
		STRUCT(CCppNativeName, Struct)

		String codeName;

		CCppNativeName(String codeName) : codeName(codeName) {}
	};
}    // namespace Rift
