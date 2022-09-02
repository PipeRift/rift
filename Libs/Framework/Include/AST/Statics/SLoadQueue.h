// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	// Keeps a list of entities to load from disk
	struct SLoadQueue : public p::Struct
	{
		STRUCT(SLoadQueue, p::Struct)

		TArray<Id> pendingSyncLoad;
		TArray<Id> pendingAsyncLoad;
	};
}    // namespace rift::AST
