// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	enum class RiftType : p::u8
	{
		None   = 0,
		Class  = 1 << 0,
		Struct = 1 << 1,
		Static = 1 << 2
	};
	P_DEFINE_FLAG_OPERATORS(RiftType);


	struct CType : public p::Struct
	{
		STRUCT(CType, p::Struct)
	};
}    // namespace rift::AST
