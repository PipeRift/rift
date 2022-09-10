// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Id.h"
#include "AST/Tree.h"

#include <Pipe/Core/Span.h>
#include <Pipe/ECS/Filtering.h>


// NOTE: In expression graphs, the Link Id is the Input Pin Id
namespace rift::AST
{
	bool CanConnectExpr(TAccessRef<CExprInputs, CExprOutputs, CExprTypeId> access,
	    ExprOutput output, ExprInput input);

	bool TryConnectExpr(TAccessRef<TWrite<CExprInputs>, CExprOutputs, CExprTypeId> access,
	    ExprOutput output, ExprInput input);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool DisconnectExpr(Tree& ast, ExprInput input);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllExprDeep(Tree& ast, TSpan<const Id> ids, bool ignoreRoot = false);

	bool RemoveExprInputPin(TAccessRef<CExprInputs, TWrite<CInvalid>> access, ExprInput id);
	bool RemoveExprOutputPin(TAccessRef<CExprOutputs, TWrite<CInvalid>> access, ExprOutput id);

	ExprInput GetExprInputFromPin(TAccessRef<CExprInputs, CChild> access, Id pinId);
	ExprOutput GetExprOutputFromPin(TAccessRef<CExprOutputs, CChild> access, Id pinId);
}    // namespace rift::AST
