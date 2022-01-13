// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Containers/Array.h>
#include <Containers/Span.h>


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::TransactionSystem
{
	void Init(AST::Tree& ast);

	void ClearTags(AST::Tree& ast);
}    // namespace Rift::TransactionSystem
