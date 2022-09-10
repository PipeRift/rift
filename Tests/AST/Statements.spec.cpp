// Copyright 2015-2022 Piperift - All rights reserved

#include <AST/Tree.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TypeUtils.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace rift;


go_bandit([]() {
	describe("AST.Statements", []() {
		it("Initializes outputs correctly", [&]() {
			AST::Tree ast;

			AST::Id functionId = AST::AddFunction({ast, AST::NoId}, "TestFunction");
			AssertThat(ast.Has<AST::CStmtOutput>(functionId), Equals(true));

			AST::Id callId = AST::AddCall({ast, AST::NoId}, functionId);
			AssertThat(ast.Has<AST::CStmtOutput>(callId), Equals(true));

			AST::Id ifId = AST::AddIf({ast, AST::NoId});
			AssertThat(ast.Has<AST::CStmtOutputs>(ifId), Equals(true));
			AssertThat(ast.Get<AST::CStmtOutputs>(ifId).pinIds.Size(), Equals(2));
		});

		it("Initializes inputs correctly", [&]() {
			AST::Tree ast;

			AST::Id functionId = AST::AddFunction({ast, AST::NoId}, "TestFunction");
			AssertThat(ast.Has<AST::CStmtInput>(functionId), Equals(false));

			AST::Id callId = AST::AddCall({ast, AST::NoId}, functionId);
			AssertThat(ast.Has<AST::CStmtInput>(callId), Equals(true));

			AST::Id ifId = AST::AddIf({ast, AST::NoId});
			AssertThat(ast.Has<AST::CStmtInput>(ifId), Equals(true));
		});

		it("Can connect with single output", [&]() {
			AST::Tree ast;

			AST::Id functionId = AST::AddFunction({ast, AST::NoId}, "TestFunction");
			AST::Id callId     = AST::AddCall({ast, AST::NoId}, functionId);
			AST::Id ifId       = AST::AddIf({ast, AST::NoId});

			AssertThat(AST::TryConnectStmt(ast, functionId, callId), Equals(true));

			// Can't connect to self
			AssertThat(AST::TryConnectStmt(ast, functionId, functionId), Equals(false));

			// Can't connect in loops
			AssertThat(AST::TryConnectStmt(ast, callId, ifId), Equals(true));
			AssertThat(AST::TryConnectStmt(ast, ifId, callId), Equals(false));

			// Can replace a connection
			AssertThat(AST::TryConnectStmt(ast, functionId, ifId), Equals(true));
		});

		it("Can connect with multiple outputs", [&]() {
			AST::Tree ast;

			AST::Id functionId = AST::AddFunction({ast, AST::NoId}, "TestFunction");

			AST::Id ifId    = AST::AddIf({ast, AST::NoId});
			AST::Id call1Id = AST::AddCall({ast, AST::NoId}, functionId);
			AST::Id call2Id = AST::AddCall({ast, AST::NoId}, functionId);

			AssertThat(
			    AST::TryConnectStmt(ast, ast.Get<AST::CStmtOutputs>(ifId).pinIds[0], call2Id),
			    Equals(true));
			AssertThat(ast.Get<AST::CStmtOutputs>(ifId).linkInputNodes[0], Equals(call2Id));

			// Can replace a connection
			AssertThat(
			    AST::TryConnectStmt(ast, ast.Get<AST::CStmtOutputs>(ifId).pinIds[0], call1Id),
			    Equals(true));
			AssertThat(ast.Get<AST::CStmtOutputs>(ifId).linkInputNodes[0], Equals(call1Id));

			// Can connect to a different pin
			AssertThat(
			    AST::TryConnectStmt(ast, ast.Get<AST::CStmtOutputs>(ifId).pinIds[1], call2Id),
			    Equals(true));
			AssertThat(ast.Get<AST::CStmtOutputs>(ifId).linkInputNodes[1], Equals(call2Id));
		});
	});
});
