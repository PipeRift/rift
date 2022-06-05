// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Reflection/Reflection.h>
#include <Reflection/Struct.h>


namespace rift::Compiler
{
	using namespace p;


	struct Config : public p::Struct
	{
		STRUCT(Config, p::Struct)

		String buildMode{"Release"};

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(AST::Tree& ast);
	};
}    // namespace rift::Compiler
