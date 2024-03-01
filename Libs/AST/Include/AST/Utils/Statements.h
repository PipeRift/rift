// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/Statements.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "AST/Utils/Statements.h"

#include <PipeArrays.h>
#include <PipeECS.h>


// NOTE: In statement graphs, the Link Id is the Input Node Id
namespace rift::ast
{
	bool CanConnectStmt(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	bool TryConnectStmt(Tree& ast, Id outputPin, Id inputNode);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool DisconnectStmtLink(Tree& ast, Id linkId);
	bool DisconnectStmtFromPrevious(Tree& ast, Id inputPin);
	bool DisconnectStmtFromNext(Tree& ast, Id outputPin, Id outputNode);
	bool DisconnectStmtFromNext(Tree& ast, Id outputPin);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllStmtDeep(Tree& ast, p::TView<const Id> ids, bool ignoreRoot = false);

	// TODO
	/** Check that a and b are connected (in any direction) */
	// bool AreNodesConnected(const Tree& ast, Id outputNode, Id inputNode);
	// bool ArePinsConnected(const Tree& ast, Id outputPin, Id inputPin);
	// bool IsOutputPinConnected(const Tree& ast, Id outputPin);
	// bool IsOutputPinConnected(const Tree& ast, Id outputPin, Id outputNode);
	// bool IsInputPinConnected(const Tree& ast, Id inputPin);
	// bool IsInputPinConnected(const Tree& ast, Id inputPin /*unused*/, Id inputNode);

	/** Look for invalid ids and set them to NoId */
	void CleanInvalidStmtIds(Tree& ast);

	// If two pins were to be connected, would they create a loop?
	bool WouldStmtLoop(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	Id GetPreviousStmt(p::TAccessRef<CStmtInput> access, Id stmtId);
	void GetPreviousStmts(
	    p::TAccessRef<CStmtInput> access, p::TView<const Id> stmtIds, p::TArray<Id>& prevStmtIds);
	p::TView<Id> GetNextStmts(p::TAccessRef<CStmtOutputs> access, Id stmtId);
	void GetNextStmts(
	    p::TAccessRef<CStmtOutputs> access, p::TView<const Id> stmtIds, p::TArray<Id>& nextStmtIds);

	void GetStmtChain(p::TAccessRef<CStmtOutput, CStmtOutputs> access, Id firstStmtId,
	    p::TArray<Id>& stmtIds, Id& splitStmtId);
}    // namespace rift::ast
