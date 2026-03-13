// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeArrays.h>
#include <PipeECS.h>


// NOTE: In expression graphs, the Link Id is the Input Pin Id
namespace rift::ast
{
	bool CanConnectExpr(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    ExprOutput output, ExprInput input);

	bool TryConnectExpr(p::TIdScopeRef<p::Writes<CExprInputs>, CExprOutputs, CExprTypeId> scope,
	    ExprOutput output, ExprInput input);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool DisconnectExpr(Tree& ast, ExprInput input);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllExprDeep(Tree& ast, p::TView<const Id> ids, bool ignoreRoot = false);

	bool RemoveExprInputPin(p::TIdScopeRef<p::Writes<CInvalid>, CExprInputs> scope, ExprInput id);
	bool RemoveExprOutputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprOutputs> scope, ExprOutput id);

	ExprInput GetExprInputFromPin(p::TIdScopeRef<CExprInputs, CChild> scope, Id pinId);
	ExprOutput GetExprOutputFromPin(p::TIdScopeRef<CExprOutputs, CChild> scope, Id pinId);
}    // namespace rift::ast
