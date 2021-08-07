// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <CoreObject.h>


namespace Rift
{
	// Contains loaded string data from disk
	struct CStringLoadUnique : public Struct
	{
		STRUCT(CStringLoadUnique, Struct)

		// This buffers are always in sync with size
		// They bind by array index an Id, path and loaded string
		TArray<AST::Id> entities;
		TArray<Path> paths;
		TArray<String> strings;
	};
}    // namespace Rift
