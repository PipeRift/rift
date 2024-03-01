// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Module.h>


namespace rift
{
	namespace ast
	{
		class Tree;
	}

	class NativeBindingModule : public Module
	{
		P_CLASS(NativeBindingModule, Module)

		void Load() override;
		void SyncIncludes(ast::Tree& ast);
	};
}    // namespace rift
