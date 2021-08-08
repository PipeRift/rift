// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	enum class LoadMode : u8
	{
		Sync,
		Async
	};

	// Keeps a list of entities to load from disk
	struct CLoadQueueUnique : public Struct
	{
		STRUCT(CLoadQueueUnique, Struct)

		TArray<AST::Id> pendingSyncLoad;
		TArray<AST::Id> pendingAsyncLoad;
	};
}    // namespace Rift
