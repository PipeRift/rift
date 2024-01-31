// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Core/String.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::ast
{
	// Contains loaded string data from disk
	struct SStringLoad : public Struct
	{
		P_STRUCT(SStringLoad, Struct)

		// This buffers are always in sync with size
		// They bind by array index an Id, path and loaded string
		TArray<Id> entities;
		TArray<p::Path> paths;
		TArray<String> strings;
	};
}    // namespace rift::ast
