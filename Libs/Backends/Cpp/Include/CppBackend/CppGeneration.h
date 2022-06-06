// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <PFiles/Paths.h>


namespace rift::Compiler
{
	struct Context;
}

namespace rift::Compiler::Cpp
{
	void GenerateCode(Context& context, const p::Path& generatePath);
}    // namespace rift::Compiler::Cpp
