// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Files/Paths.h>


namespace Rift::Compiler
{
	struct Context;
}

namespace Rift::Compiler::Cpp
{
	void GenerateCode(Context& context, const Path& generatePath);
}    // namespace Rift::Compiler::Cpp
