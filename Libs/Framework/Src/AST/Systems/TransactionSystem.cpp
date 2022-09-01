// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TransactionSystem.h"

#include "AST/Components/Tags/CChanged.h"


namespace rift::AST::TransactionSystem
{
	void Init(Tree& ast) {}
	void ClearTags(Tree& ast)
	{
		ast.AssurePool<CChanged>().Clear();
	}
}    // namespace rift::AST::TransactionSystem
