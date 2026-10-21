// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/Statements.h"
#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
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

	// If two pins were to be connected, would they create a loop?
	bool WouldStmtLoop(const Tree& ast, Id outputNode, Id outputPin, Id inputNode);

	Id GetPreviousStmt(p::TIdScopeRef<CStmtInput> scope, Id stmtId);
	void GetPreviousStmts(
	    p::TIdScopeRef<CStmtInput> scope, p::TView<const Id> stmtIds, p::TArray<Id>& prevStmtIds);
	p::TView<Id> GetNextStmts(p::TIdScopeRef<CStmtOutputs> scope, Id stmtId);
	void GetNextStmts(
	    p::TIdScopeRef<CStmtOutputs> scope, p::TView<const Id> stmtIds, p::TArray<Id>& nextStmtIds);

	void GetStmtChain(p::TIdScopeRef<CStmtOutput, CStmtOutputs> scope, Id firstStmtId,
	    p::TArray<Id>& stmtIds, Id& splitStmtId);
}    // namespace rift::ast
