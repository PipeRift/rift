// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Systems/TransactionSystem.h"

#include "AST/Components/Tags/CChanged.h"


namespace rift::ast::TransactionSystem
{
	void Init(Tree& ast) {}
	void ClearTags(Tree& ast)
	{
		ast.AssurePool<CChanged>().Clear();
	}
}    // namespace rift::ast::TransactionSystem
