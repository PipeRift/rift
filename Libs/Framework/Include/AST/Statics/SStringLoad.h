// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Types/Struct.h>


namespace Rift
{
	// Contains loaded string data from disk
	struct SStringLoad : public Struct
	{
		STRUCT(SStringLoad, Struct)

		// This buffers are always in sync with size
		// They bind by array index an Id, path and loaded string
		TArray<AST::Id> entities;
		TArray<Path> paths;
		TArray<String> strings;
	};
}    // namespace Rift
