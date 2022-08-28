// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CNativeModule : public p::Struct
	{
		STRUCT(CNativeModule, p::Struct)

		PROP(includePaths)
		p::TArray<p::Path> includePaths;
	};
}    // namespace rift
