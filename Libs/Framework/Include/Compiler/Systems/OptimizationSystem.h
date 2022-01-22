// Copyright 2015-2022 Piperift - All rights reserved
#pragma once


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::OptimizationSystem
{
	void PruneDisconnectedStatements(AST::Tree& ast);
	void PruneDisconnectedExpressions(AST::Tree& ast);
}    // namespace Rift::OptimizationSystem
