// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Reflection/Reflection.h>
#include <Reflection/Struct.h>


namespace Rift::Compiler
{
	using namespace Pipe;


	struct Config : public Pipe::Struct
	{
		STRUCT(Config, Pipe::Struct)

		String buildMode{"Release"};

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(AST::Tree& ast);
	};
}    // namespace Rift::Compiler
