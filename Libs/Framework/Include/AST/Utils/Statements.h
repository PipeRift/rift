// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "AST/Utils/Statements.h"

#include <Core/Span.h>
#include <ECS/Filtering.h>


// NOTE: In statement graphs, the Link Id is the Input Node Id
namespace rift::AST::Statements
{
	bool CanConnect(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	bool TryConnect(Tree& ast, AST::Id outputPin, AST::Id inputNode);
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

	// If two pins were to be connected, would they create a loop?
	bool WouldLoop(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	Id GetConnectedToInput(TAccessRef<CStmtInput> access, Id node);
	void GetConnectedToInputs(
	    TAccessRef<CStmtInput> access, TSpan<const Id> nodes, TArray<Id>& ids);
	TSpan<Id> GetConnectedToOutputs(TAccessRef<CStmtOutputs> access, Id node);
	void GetConnectedToOutputs(
	    TAccessRef<CStmtOutputs> access, TSpan<const Id> nodes, TArray<Id>& ids);

	void GetChain(TAccessRef<CStmtOutput, CStmtOutputs> access, Id firstStmtId, TArray<Id>& stmtIds,
	    Id& splitStmtId);
}    // namespace rift::AST::Statements
