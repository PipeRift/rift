// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Id.h>
#include <Files/Paths.h>


namespace rift::Compiler
{
	struct Context;
}

namespace rift::Compiler::Cpp
{
	void GenerateCMake(Context& context, const pipe::Path& codePath);
}    // namespace rift::Compiler::Cpp
