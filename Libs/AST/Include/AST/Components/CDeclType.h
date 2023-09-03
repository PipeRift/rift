// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct CDeclType : public p::Struct
	{
		P_STRUCT(CDeclType, p::Struct)

		P_PROP(typeId)
		p::Tag typeId;
	};
}    // namespace rift::AST
