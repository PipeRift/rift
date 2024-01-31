// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/Statements.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "AST/Utils/Statements.h"

#include <PipeArrays.h>
#include <PipeECS.h>



// NOTE: In statement graphs, the Link Id is the Input Node Id
namespace rift::AST
{
	bool CanConnectStmt(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	bool TryConnectStmt(Tree& ast, AST::Id outputPin, AST::Id inputNode);
	// Disconnects a particular link. (Note: link ids are the same as input nodes)
	bool DisconnectStmtLink(Tree& ast, AST::Id linkId);
	bool DisconnectStmtFromPrevious(Tree& ast, AST::Id inputPin);
	bool DisconnectStmtFromNext(Tree& ast, AST::Id outputPin, AST::Id outputNode);
	bool DisconnectStmtFromNext(Tree& ast, AST::Id outputPin);

	/**
	 * @brief Disconnects all inputs and outputs from this ids and the children nodes
	 *
	 * @param ids
	 * @param ignoreRoot ignore ids's inputs and outputs and only remove from children
	 */
	void DisconnectAllStmtDeep(Tree& ast, TView<const AST::Id> ids, bool ignoreRoot = false);

	// TODO
	/** Check that a and b are connected (in any direction) */
	// bool AreNodesConnected(const Tree& ast, AST::Id outputNode, AST::Id inputNode);
	// bool ArePinsConnected(const Tree& ast, AST::Id outputPin, AST::Id inputPin);
	// bool IsOutputPinConnected(const Tree& ast, AST::Id outputPin);
	// bool IsOutputPinConnected(const Tree& ast, AST::Id outputPin, AST::Id outputNode);
	// bool IsInputPinConnected(const Tree& ast, AST::Id inputPin);
	// bool IsInputPinConnected(const Tree& ast, AST::Id inputPin /*unused*/, AST::Id inputNode);

	/** Look for invalid ids and set them to NoId */
	void CleanInvalidStmtIds(Tree& ast);

	// If two pins were to be connected, would they create a loop?
	bool WouldStmtLoop(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	Id GetPreviousStmt(TAccessRef<CStmtInput> access, Id stmtId);
	void GetPreviousStmts(
	    TAccessRef<CStmtInput> access, TView<const Id> stmtIds, TArray<Id>& prevStmtIds);
	TView<Id> GetNextStmts(TAccessRef<CStmtOutputs> access, Id stmtId);
	void GetNextStmts(
	    TAccessRef<CStmtOutputs> access, TView<const Id> stmtIds, TArray<Id>& nextStmtIds);

	void GetStmtChain(TAccessRef<CStmtOutput, CStmtOutputs> access, Id firstStmtId,
	    TArray<Id>& stmtIds, Id& splitStmtId);
}    // namespace rift::AST
