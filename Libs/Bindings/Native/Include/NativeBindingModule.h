// Copyright 2015-2022 Piperift - All rights reserved

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

		void SyncIncludes(AST::Tree& ast);
	};
}    // namespace rift
