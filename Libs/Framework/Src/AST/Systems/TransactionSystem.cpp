// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TransactionSystem.h"

#include "AST/Components/Tags/CChanged.h"


namespace Rift::TransactionSystem
{
	void Init(AST::Tree& ast) {}
	void ClearTags(AST::Tree& ast)
	{
		ast.AssurePool<CChanged>().Reset();
	}
}    // namespace Rift::TransactionSystem