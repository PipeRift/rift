// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Pipe/Reflect/Reflection.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	using namespace p;


	struct CompilerConfig : public p::Struct
	{
		P_STRUCT(CompilerConfig, p::Struct)

		String buildMode{"Release"};

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(AST::Tree& ast);
	};
}    // namespace rift
