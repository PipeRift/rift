// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	struct CCppNativeName : public Pipe::Struct
	{
		STRUCT(CCppNativeName, Pipe::Struct)

		String codeName;

		CCppNativeName(String codeName) : codeName(codeName) {}
	};
}    // namespace Rift
