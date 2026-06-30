// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>


namespace rift::editor
{
	struct ArenaDebugger
	{
		bool open = false;


		ArenaDebugger();

		void Draw(ast::Tree& ast);
	};
}    // namespace rift::editor
