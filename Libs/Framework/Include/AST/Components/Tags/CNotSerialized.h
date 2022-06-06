// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <PRefl/Struct.h>


namespace rift
{
	// Mark an entity as not serialized.
	// Will be entirely ignored by the serializer
	struct CNotSerialized : public p::Struct
	{
		STRUCT(CNotSerialized, p::Struct)
	};
}    // namespace rift
