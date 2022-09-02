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
namespace rift::AST::Expressions
{
	bool CanConnect(
	    TAccessRef<CExprInputs, CExprOutputs, CExprTypeId> access, OutputId output, InputId input);

	bool TryConnect(TAccessRef<TWrite<CExprInputs>, CExprOutputs, CExprTypeId> access,
	    OutputId output, InputId input);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool Disconnect(Tree& ast, InputId input);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllDeep(Tree& ast, TSpan<const Id> ids, bool ignoreRoot = false);

	bool RemoveInputPin(TAccessRef<CExprInputs, TWrite<CInvalid>> access, InputId id);
	bool RemoveOutputPin(TAccessRef<CExprOutputs, TWrite<CInvalid>> access, OutputId id);

	InputId InputFromPinId(TAccessRef<CExprInputs, CChild> access, Id pinId);
	OutputId OutputFromPinId(TAccessRef<CExprOutputs, CChild> access, Id pinId);
}    // namespace rift::AST::Expressions
