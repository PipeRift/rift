// Copyright 2015-2023 Piperift - All rights reserved
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
