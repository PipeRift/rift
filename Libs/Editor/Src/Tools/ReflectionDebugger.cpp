// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Tools/ReflectionDebugger.h"

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
	ReflectionDebugger::ReflectionDebugger() {}

	void ReflectionDebugger::Draw(ast::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		auto& dbgContext = ast.GetOrSetStatic<p::DebugContext>();
		dbgContext.ctx   = &ast;
		if (p::BeginDebug(dbgContext))
		{
			p::DrawReflection("Reflection", &open);
			p::EndDebug();
		}
	}
}    // namespace rift::editor
