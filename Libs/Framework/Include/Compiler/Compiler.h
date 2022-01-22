// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"


namespace Rift::Compiler
{
	enum class EBackend : u8
	{
		Cpp,
		LLVM
	};


	void Build(AST::Tree& tree, const Config& config, EBackend backend);
}    // namespace Rift::Compiler
