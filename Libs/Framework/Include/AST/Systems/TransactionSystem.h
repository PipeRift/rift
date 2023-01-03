// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Pipe/Core/Array.h>
#include <Pipe/Core/Span.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::AST::TransactionSystem
{
	void Init(Tree& ast);

	void ClearTags(Tree& ast);
}    // namespace rift::AST::TransactionSystem