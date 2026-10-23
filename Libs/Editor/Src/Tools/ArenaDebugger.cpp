// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Tools/ArenaDebugger.h"

#include <AST/Components/Statements.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <Misc/PipeDebug.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


namespace rift::editor
{
	ArenaDebugger::ArenaDebugger() {}

	void ArenaDebugger::Draw(ast::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		auto& dbgContext = ast.GetOrSetStatic<p::DebugContext>();
		dbgContext.ctx   = &ast;
		if (p::BeginDebug(dbgContext))
		{
			p::DrawMemory("Memory", &open);
			p::EndDebug();
		}
	}
}    // namespace rift::editor
