// Copyright 2015-2022 Piperift - All rights reserved
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


namespace rift::AST
{
	struct Tree;
}

namespace rift::FunctionsSystem
{
	struct CTmpInvalidKeep
	{};

	void Init(AST::Tree& ast);
	void ResolveCallFunctionIds(
	    TAccessRef<TWrite<CExprCallId>, CExprCall, CDeclFunction, CNamespace, CParent, CChild>
	        access);
	void PushInvalidPinsBack(
	    TAccessRef<TWrite<CExprInputs>, TWrite<CExprOutputs>, CInvalid> access);

	// Marks calls referencing dirty functions as dirty theirselfs
	void PropagateDirtyIntoCalls(AST::Tree& ast);
	void SyncCallPinsFromFunction(AST::Tree& ast);
	using InvalidDisconnectedPinAccess =
	    TAccessRef<CInvalid, CExprInputs, TWrite<CTmpInvalidKeep>, TWrite<CChild>, TWrite<CParent>>;
	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess access);
	void ClearAddedTags(AST::Tree& ast);
}    // namespace rift::FunctionsSystem
