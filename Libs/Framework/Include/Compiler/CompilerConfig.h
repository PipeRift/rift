// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"

#include <Reflection/Reflection.h>
#include <Types/Struct.h>


namespace Rift::Compiler
{
	struct Config : public Struct
	{
		STRUCT(Config, Struct)

		String buildMode{"Release"};

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(AST::Tree& ast);
	};
}    // namespace Rift::Compiler
