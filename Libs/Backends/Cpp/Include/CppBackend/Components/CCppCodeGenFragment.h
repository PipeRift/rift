// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CCppCodeGenFragment : public p::Struct
	{
		STRUCT(CCppCodeGenFragment, p::Struct)

		String code;
	};
}    // namespace rift
