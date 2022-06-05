// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Files/Paths.h>


namespace rift::Compiler
{
	struct Context;
}

namespace rift::Compiler::Cpp
{
	void GenerateCode(Context& context, const pipe::Path& generatePath);
}    // namespace rift::Compiler::Cpp
