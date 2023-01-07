// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

namespace rift::AST
{
	struct Tree;
}

namespace rift::OptimizationSystem
{
	void PruneDisconnectedStatements(AST::Tree& ast);
	void PruneDisconnectedExpressions(AST::Tree& ast);
}    // namespace rift::OptimizationSystem
