// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	// Mark an entity as not serialized.
	// Will be entirely ignored by the serializer
	struct CNotSerialized : public pipe::Struct
	{
		STRUCT(CNotSerialized, pipe::Struct)
	};
}    // namespace rift
