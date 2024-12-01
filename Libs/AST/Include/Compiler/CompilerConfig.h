// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <PipePlatform.h>
#include <PipeReflect.h>


namespace rift
{
	enum class OptimizationLevel : p::u8
	{
		Zero = 0,    // Only register allocator and machine code generator work
		One = 1,    // Additional code selection task. On this level more compact and faster code is
		            // generated than on zero level with practically on the same speed
		Two = 2,    // (Default) Additional common sub-expression elimination and sparse conditional
		            // constant propagation. This level is valuable if you generate bad input MIR
		            // code with a lot of redundancy and constants. The generation speed on level 1
		            // is ~50% faster than on level 2
		Three = 3,    // Additional register renaming and loop invariant code motion. The generation
		              // speed on level 2 is ~50% faster than on level 3
	};

	struct CompilerConfig
	{
		P_STRUCT(CompilerConfig)

		OptimizationLevel optimization = OptimizationLevel::Two;

		bool debug   = true;
		bool verbose = false;

		p::String buildPath;
		p::String intermediatesPath;
		p::String binariesPath;

		void Init(ast::Tree& ast);
	};
}    // namespace rift
P_ENUM(rift::OptimizationLevel);
