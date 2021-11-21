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
	struct STypeList : public Struct
	{
		STRUCT(STypeList, Struct)

		TMap<Name, AST::Id> types;
	};
}    // namespace Rift
