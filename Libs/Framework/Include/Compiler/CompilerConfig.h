// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Pipe/Reflect/Reflection.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::compiler
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
}    // namespace rift::compiler
