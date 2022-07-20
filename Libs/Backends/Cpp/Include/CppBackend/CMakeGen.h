// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Id.h>
#include <Compiler/Compiler.h>
#include <Pipe/Files/Paths.h>


namespace rift::compiler::Cpp
{
	void GenerateCMake(Compiler& compiler, const p::Path& codePath);
}    // namespace rift::compiler::Cpp
