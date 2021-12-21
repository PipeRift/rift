// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "AST/Types.h"

#include <Containers/Span.h>


namespace Rift::AST::StatementGraph
{
	bool CanConnect(const Tree& ast, AST::Id a, AST::Id b);

	void Connect(Tree& ast, AST::Id outputNode, AST::Id outputPin, AST::Id inputNode);
	void Connect(Tree& ast, AST::Id outputPin, AST::Id inputPin);
	// Disconnects a particular edge. (Note: edge ids are the same as input nodes)
	bool Disconnect(Tree& ast, AST::Id edgeId);

	/** Disconnects all inputs and outputs from a list of ids */
	void DisconnectAll(Tree& ast, TSpan<const AST::Id> ids);
	void DisconnectAllInputs(Tree& ast, TSpan<const AST::Id> outputs);
	void DisconnectAllOutputs(Tree& ast, TSpan<const AST::Id> inputs);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllDeep(Tree& ast, TSpan<const AST::Id> ids, bool ignoreRoot = false);


	AST::Id GetInput(const Tree& ast, AST::Id output);
	TArray<Id> GetInputs(const Tree& ast, TSpan<Id> outputs);

	const TArray<AST::Id> GetOutputs(const Tree& ast, AST::Id input);
	TArray<Id> GetOutputs(const Tree& ast, TSpan<Id> inputs);

	/** Check that a and b are connected (in any direction) */
	bool IsConnected(const Tree& ast, AST::Id a, AST::Id b);

	/** Look for invalid ids around the ast and remove them from the graph */
	void CleanInvalidIds(Tree& ast);
}    // namespace Rift::AST::StatementGraph
