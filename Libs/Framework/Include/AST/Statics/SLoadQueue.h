// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	// Keeps a list of entities to load from disk
	struct SLoadQueue : public Struct
	{
		STRUCT(SLoadQueue, Struct)

		TArray<AST::Id> pendingSyncLoad;
		TArray<AST::Id> pendingAsyncLoad;
	};
}    // namespace Rift
