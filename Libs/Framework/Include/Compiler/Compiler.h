// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/CompilerConfig.h"


namespace rift::Compiler
{
	void Build(AST::Tree& tree, const Config& config, TPtr<Backend> backend);

	inline void Build(AST::Tree& ast, const Config& config, refl::ClassType* backendType)
	{
		if (backendType)
		{
			TOwnPtr<Backend> backend = MakeOwned<Backend>(backendType);
			Build(ast, config, backend);
		}
	}

	template<typename T>
	void Build(AST::Tree& ast, const Config& config)
	{
		Build(ast, config, T::GetStaticType());
	}
}    // namespace rift::Compiler
