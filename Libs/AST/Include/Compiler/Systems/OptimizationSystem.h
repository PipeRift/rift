// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

namespace rift::ast
{
	struct Tree;
}

namespace rift::OptimizationSystem
{
	void PruneDisconnectedStatements(ast::Tree& ast);
	void PruneDisconnectedExpressions(ast::Tree& ast);
}    // namespace rift::OptimizationSystem
