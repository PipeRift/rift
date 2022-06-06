// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <PCore/Array.h>
#include <PCore/Span.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::TransactionSystem
{
	void Init(AST::Tree& ast);

	void ClearTags(AST::Tree& ast);
}    // namespace rift::TransactionSystem
