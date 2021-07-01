// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include <Files/Paths.h>


namespace Rift::Compiler
{
	struct Context;
}

namespace Rift::Compiler::Cpp
{
	void GenerateCMake(Context& context, const Path& generatePath);
}    // namespace Rift::Compiler::Cpp
