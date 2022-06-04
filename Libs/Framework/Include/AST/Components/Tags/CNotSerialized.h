// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	// Mark an entity as not serialized.
	// Will be entirely ignored by the serializer
	struct CNotSerialized : public Pipe::Struct
	{
		STRUCT(CNotSerialized, Pipe::Struct)
	};
}    // namespace Rift
