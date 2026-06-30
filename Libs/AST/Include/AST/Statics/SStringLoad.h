// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"

#include <Pipe/Core/String.h>
#include <Pipe/Files/Paths.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// Contains loaded string data from disk
	struct SStringLoad
	{
		P_STRUCT(SStringLoad)

		// This buffers are always in sync with size
		// They bind by array index an Id, path and loaded string
		p::TArray<p::Id> entities;
		p::TArray<p::String> paths;
		p::TArray<p::String> strings;
	};
}    // namespace rift::ast
