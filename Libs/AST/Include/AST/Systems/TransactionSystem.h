// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"


namespace rift::ast
{
	struct Tree;
}

namespace rift::ast::TransactionSystem
{
	void Init(Tree& ast);

	void ClearTags(Tree& ast);
}    // namespace rift::ast::TransactionSystem
