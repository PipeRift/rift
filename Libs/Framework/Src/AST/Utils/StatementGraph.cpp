// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/StatementGraph.h"


namespace Rift::AST::StatementGraph
{
	void Connect(Tree& ast, TSpan<const AST::Id> outputs, AST::Id input) {}
	void DisconnectAllInputs(Tree& ast, TSpan<const AST::Id> outputs) {}
}    // namespace Rift::AST::StatementGraph
