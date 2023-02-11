// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	using namespace p::core;

	static constexpr StringView moduleFilename = "__module__.rf";

	struct CModule : public p::Struct
	{
		STRUCT(CModule, p::Struct)

		PROP(dependencies)
		TArray<Tag> dependencies;
	};
}    // namespace rift::AST
