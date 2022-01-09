// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Containers/Array.h>
#include <Containers/Span.h>


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::FunctionsSystem
{
	void Init(AST::Tree& ast);
	void ResolveCallFunctionIds(AST::Tree& ast);
	void SyncCallArguments(AST::Tree& ast);
	void ClearAddedTags(AST::Tree& ast);
}    // namespace Rift::FunctionsSystem
