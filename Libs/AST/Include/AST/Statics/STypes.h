// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::ast
{
	struct STypes : public p::Struct
	{
		P_STRUCT(STypes, p::Struct)

		p::TMap<p::Tag, Id> typesByName;
		// TODO: Use StringView to point to CFileRef component's path.
		// Current TMap lookup of stringviews seems unconsistent
		p::TMap<p::Tag, Id> typesByPath;
	};
}    // namespace rift::ast
