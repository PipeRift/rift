// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace Rift
{
	// Keeps a list of entities to load from disk
	struct SLoadQueue : public Pipe::Struct
	{
		STRUCT(SLoadQueue, Pipe::Struct)

		TArray<AST::Id> pendingSyncLoad;
		TArray<AST::Id> pendingAsyncLoad;
	};
}    // namespace Rift
