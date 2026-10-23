// Copyright 2015-2026 Piperift. All Rights Reserved.

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
	public:
		using Super = Module;
		P_CLASS(NativeBindingModule)

		void Load() override;
		void SyncIncludes(ast::Tree& ast);
	};
}    // namespace rift
