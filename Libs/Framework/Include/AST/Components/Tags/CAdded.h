// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>


namespace Rift
{
	template<typename C>
	struct CAdded : public Struct
	{
		STRUCT(CAdded<C>, Struct, Struct_NotSerialized)
	};
}    // namespace Rift
