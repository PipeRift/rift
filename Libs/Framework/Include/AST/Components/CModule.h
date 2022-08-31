// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CModule : public p::Struct
	{
		STRUCT(CModule, p::Struct)

		PROP(dependencies)
		p::TArray<p::Name> dependencies;
	};
}    // namespace rift
