// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Tree.h"
#include "PipeReflect.h"

#include <UI/UI.h>


namespace rift::editor
{
	using namespace p;


	struct ReflectionDebugger
	{
		bool open = false;

		ReflectionDebugger();
		void Draw(ast::Tree& ast);
	};
}    // namespace rift::editor
