// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Module.h>


namespace rift
{
	namespace AST
	{
		class Tree;
	}

	class NativeBindingModule : public Module
	{
		CLASS(NativeBindingModule, Module)

		void Load() override;
		void SyncIncludes(AST::Tree& ast);
	};
}    // namespace rift
