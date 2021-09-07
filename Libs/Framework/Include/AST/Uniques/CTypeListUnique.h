// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>
#include <Misc/Guid.h>


namespace Rift
{
	enum class LoadMode : u8
	{
		Sync,
		Async
	};

	// Keeps a list of entities to load from disk
	struct CTypeListUnique : public Struct
	{
		STRUCT(CTypeListUnique, Struct)

		TMap<Guid, AST::Id> types;
	};
}    // namespace Rift
