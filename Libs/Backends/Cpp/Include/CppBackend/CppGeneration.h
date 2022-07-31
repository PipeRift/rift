// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Pipe/Files/Paths.h>


namespace rift::compiler
{
	struct Compiler;
}

namespace rift::compiler::Cpp
{
	void GenerateCode(Compiler& compiler, const p::Path& generatePath);
}    // namespace rift::compiler::Cpp
