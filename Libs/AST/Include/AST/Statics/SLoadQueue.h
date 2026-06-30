// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"

#include <PipeReflect.h>


namespace rift::ast
{
	// Keeps a list of entities to load from disk
	struct SLoadQueue
	{
		P_STRUCT(SLoadQueue)

		p::TArray<Id> pendingSyncLoad;
		p::TArray<Id> pendingAsyncLoad;
	};
}    // namespace rift::ast
