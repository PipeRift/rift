// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	using namespace p::core;

	struct STypes : public Struct
	{
		STRUCT(STypes, Struct)

		TMap<Tag, Id> typesByName;
		// TODO: Use StringView to point to CFileRef component's path.
		// Current TMap lookup of stringviews seems unconsistent
		TMap<Tag, Id> typesByPath;
	};
}    // namespace rift::AST
