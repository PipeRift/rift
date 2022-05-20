// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/Tags/CInvalid.h"
#include "AST/Filtering.h"
#include "AST/Tree.h"
#include "AST/Types.h"
#include "AST/Utils/Expressions.h"

#include <Containers/Span.h>


// NOTE: In expression graphs, the Link Id is the Input Pin Id
namespace Rift::AST::Expressions
{
	bool CanConnect(TAccessRef<CExprInputs, CExprOutputs> access, OutputId output, InputId input);

	bool TryConnect(
	    TAccessRef<TWrite<CExprInputs>, CExprOutputs> access, OutputId output, InputId input);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool Disconnect(Tree& ast, InputId input);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllDeep(Tree& ast, TSpan<const AST::Id> ids, bool ignoreRoot = false);

	bool RemoveInputPin(
	    TAccessRef<TWrite<CExprInputs>, TWrite<CInvalid>, CChild> access, InputId id);
	bool RemoveOutputPin(
	    TAccessRef<TWrite<CExprOutputs>, TWrite<CInvalid>, CChild> access, OutputId id);

	InputId InputFromPinId(TAccessRef<CExprInputs, CChild> access, AST::Id pinId);
	OutputId OutputFromPinId(TAccessRef<CExprOutputs, CChild> access, AST::Id pinId);
}    // namespace Rift::AST::Expressions
