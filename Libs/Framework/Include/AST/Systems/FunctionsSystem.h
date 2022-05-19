// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExprInputs.h"
#include "AST/Components/Tags/CInvalid.h"
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
	void PropagateDirtyIntoCalls(AST::Tree& ast);
	void SyncCallArguments(AST::Tree& ast);
	void RemoveInvalidDisconnectedArgs(TAccessRef<CInvalid, CExprInputs> access);
	void ClearAddedTags(AST::Tree& ast);
}    // namespace Rift::FunctionsSystem
