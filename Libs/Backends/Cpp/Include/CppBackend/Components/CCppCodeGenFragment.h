// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	struct CCppCodeGenFragment : public pipe::Struct
	{
		STRUCT(CCppCodeGenFragment, pipe::Struct)

		String code;
	};
}    // namespace rift
