// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	struct CCppCodeGenFragment : public Pipe::Struct
	{
		STRUCT(CCppCodeGenFragment, Pipe::Struct)

		String code;
	};
}    // namespace Rift
