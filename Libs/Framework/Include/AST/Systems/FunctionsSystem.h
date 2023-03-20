// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Tree.h"

#include <Pipe/Core/Array.h>
#include <Pipe/Core/Span.h>
#include <Pipe/ECS/Access.h>


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
	    TAccessRef<TWrite<CExprInputs>, TWrite<CExprOutputs>, CInvalid> access);

	// Marks calls referencing dirty functions as dirty theirselfs
	void PropagateDirtyIntoCalls(Tree& ast);
	void SyncCallPinsFromFunction(Tree& ast);
	using InvalidDisconnectedPinAccess = p::TAccessRef<CInvalid, CExprInputs,
	    TWrite<CTmpInvalidKeep>, TWrite<CChild>, TWrite<CParent>>;
	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess access);
	void ClearAddedTags(Tree& ast);
}    // namespace rift::AST::FunctionsSystem
