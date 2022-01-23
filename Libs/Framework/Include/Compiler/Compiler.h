// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/CompilerConfig.h"


namespace Rift::Compiler
{
	void Build(AST::Tree& tree, const Config& config, TPtr<Backend> backend);

	inline void Build(AST::Tree& tree, const Config& config, Refl::ClassType* backendType)
	{
		if (backendType)
		{
			TOwnPtr<Backend> backend = MakeOwned<Backend>(backendType);
			Build(tree, config, backend);
		}
	}
}    // namespace Rift::Compiler
