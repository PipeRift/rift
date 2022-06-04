// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Id.h>
#include <Files/Paths.h>


namespace Rift::Compiler
{
	struct Context;
}

namespace Rift::Compiler::Cpp
{
	void GenerateCMake(Context& context, const Pipe::Path& codePath);
}    // namespace Rift::Compiler::Cpp
