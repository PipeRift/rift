// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeECS.h>


// NOTE: In expression graphs, the Link Id is the Input Pin Id
namespace rift::ast
{
	bool CanConnectExpr(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    const ExprOutput& output, const ExprInput& input);

	bool TryConnectExpr(p::TIdScopeRef<p::Writes<CExprInputs>, CExprOutputs, CExprTypeId> scope,
	    const ExprOutput& output, ExprInput input);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool DisconnectExpr(Tree& ast, const ExprInput& input);

	bool RemoveExprInputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprInputs> scope, const ExprInput& id);
	bool RemoveExprOutputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprOutputs> scope, const ExprOutput& id);

	ExprInput GetExprInputFromPin(p::TIdScopeRef<CExprInputs, CChild> scope, Id pinId);
	ExprOutput GetExprOutputFromPin(p::TIdScopeRef<CExprOutputs, CChild> scope, Id pinId);

	// Type of an expression pin, or NoId when it has none
	Id GetPinType(p::TIdScopeRef<CExprTypeId> scope, Id pinId);
}    // namespace rift::ast
