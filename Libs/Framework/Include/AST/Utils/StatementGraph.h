// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "AST/Types.h"
#include "AST/Utils/StatementGraph.h"

#include <Containers/Span.h>


namespace Rift::AST::StatementGraph
{
	bool CanConnect(const Tree& ast, AST::Id a, AST::Id b);

	void Connect(Tree& ast, AST::Id outputNode, AST::Id outputPin, AST::Id inputNode);
	void Connect(Tree& ast, AST::Id outputPin, AST::Id inputPin);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool Disconnect(Tree& ast, AST::Id linkId);
	bool DisconnectFromInputPin(Tree& ast, AST::Id inputPin);
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin);
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin, AST::Id outputNode);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllDeep(Tree& ast, TSpan<const AST::Id> ids, bool ignoreRoot = false);

	// TODO
	/** Check that a and b are connected (in any direction) */
	// bool AreNodesConnected(const Tree& ast, AST::Id outputNode, AST::Id inputNode);
	// bool ArePinsConnected(const Tree& ast, AST::Id outputPin, AST::Id inputPin);
	// bool IsOutputPinConnected(const Tree& ast, AST::Id outputPin);
	// bool IsOutputPinConnected(const Tree& ast, AST::Id outputPin, AST::Id outputNode);
	// bool IsInputPinConnected(const Tree& ast, AST::Id inputPin);
	// bool IsInputPinConnected(const Tree& ast, AST::Id inputPin /*unused*/, AST::Id inputNode);

	/** Look for invalid ids and set them to NoId */
	void CleanInvalidIds(Tree& ast);
}    // namespace Rift::AST::StatementGraph
