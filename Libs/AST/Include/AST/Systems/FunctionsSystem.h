// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Tree.h"

#include <PipeArrays.h>
#include <PipeECS.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::AST::FunctionsSystem
{
	struct CTmpInvalidKeep
	{};

	void Init(Tree& ast);
	void ResolveCallFunctionIds(
	    p::TAccessRef<p::TWrite<CExprCallId>, CExprCall, CDeclFunction, CNamespace, CParent, CChild>
	        access);
	void PushInvalidPinsBack(
	    p::TAccessRef<p::TWrite<CExprInputs>, p::TWrite<CExprOutputs>, CInvalid> access);

	// Marks calls referencing dirty functions as dirty theirselfs
	void PropagateDirtyIntoCalls(Tree& ast);
	void SyncCallPinsFromFunction(Tree& ast);
	using InvalidDisconnectedPinAccess = p::TAccessRef<CInvalid, CExprInputs,
	    p::TWrite<CTmpInvalidKeep>, p::TWrite<CChild>, p::TWrite<CParent>>;
	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess access);
	void ClearAddedTags(Tree& ast);
}    // namespace rift::AST::FunctionsSystem
