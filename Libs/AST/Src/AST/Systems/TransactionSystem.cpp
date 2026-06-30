// Copyright 2015-2026 Piperift. All Rights Reserved.

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
