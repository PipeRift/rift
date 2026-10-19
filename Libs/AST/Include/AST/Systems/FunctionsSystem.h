// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Statements.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct Tree;
}

namespace rift::ast::FunctionsSystem
{
	struct CTmpInvalidKeep
	{};

	void Init(Tree& ast);
	void ResolveCallFunctionIds(p::TIdScopeRef<p::Writes<CExprCallId>, CExprCall, CDeclFunction,
	    CNamespace, CParent, CChild, CModule, CDeclNative>
	        scope);
	void PushInvalidPinsBack(p::TIdScopeRef<p::Writes<CExprInputs, CExprOutputs>, CInvalid> scope);

	// Marks calls referencing dirty functions as dirty theirselfs
	void PropagateDirtyIntoCalls(Tree& ast);
	// Marks every resolved call as dirty. Used by head-less builds, where no
	// editing transaction tagged functions as changed.
	void MarkAllCallsDirty(Tree& ast);
	void SyncCallPinsFromFunction(Tree& ast);
	// Resolves the function a statement belongs to by walking the statement chain back to
	// the function declaration node. Returns NoId when the statement isn't connected to one.
	Id FindOwningFunction(p::TIdScopeRef<CStmtInput, CDeclFunction> scope, Id stmtId);
	// Populates return statement pins from the outputs of the function they're connected to
	void SyncReturnPinsFromFunction(Tree& ast);
	// Repairs invalid graph connectivity (e.g. mismatched pin/link counts) so
	// drawing and code generation can continue instead of failing. Logs a
	// warning for every node it has to repair.
	void HealGraph(Tree& ast);
	using InvalidDisconnectedPinAccess =
	    p::TIdScopeRef<p::Writes<CTmpInvalidKeep, CChild, CParent>, CInvalid, CExprInputs>;
	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess scope);
	void ClearAddedTags(Tree& ast);
}    // namespace rift::ast::FunctionsSystem
