// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	struct CCppNativeName : public p::Struct
	{
		STRUCT(CCppNativeName, p::Struct)

		String codeName;

		CCppNativeName(String codeName) : codeName(codeName) {}
	};
}    // namespace rift
