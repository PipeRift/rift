// Copyright 2015-2023 Piperift - All rights reserved

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
			ast::Tree ast;

			ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
			AssertThat(ast.Has<ast::CStmtOutput>(functionId), Equals(true));

			ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
			AssertThat(ast.Has<ast::CStmtOutput>(callId), Equals(true));

			ast::Id ifId = ast::AddIf({ast, ast::NoId});
			AssertThat(ast.Has<ast::CStmtOutputs>(ifId), Equals(true));
			AssertThat(ast.Get<ast::CStmtOutputs>(ifId).pinIds.Size(), Equals(2));
		});

		it("Initializes inputs correctly", [&]() {
			ast::Tree ast;

			ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
			AssertThat(ast.Has<ast::CStmtInput>(functionId), Equals(false));

			ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
			AssertThat(ast.Has<ast::CStmtInput>(callId), Equals(true));

			ast::Id ifId = ast::AddIf({ast, ast::NoId});
			AssertThat(ast.Has<ast::CStmtInput>(ifId), Equals(true));
		});

		it("Can connect with single output", [&]() {
			ast::Tree ast;

			ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
			ast::Id callId     = ast::AddCall({ast, ast::NoId}, functionId);
			ast::Id ifId       = ast::AddIf({ast, ast::NoId});

			AssertThat(ast::TryConnectStmt(ast, functionId, callId), Equals(true));

			// Can't connect to self
			AssertThat(ast::TryConnectStmt(ast, functionId, functionId), Equals(false));

			// Can't connect in loops
			AssertThat(ast::TryConnectStmt(ast, callId, ifId), Equals(true));
			AssertThat(ast::TryConnectStmt(ast, ifId, callId), Equals(false));

			// Can replace a connection
			AssertThat(ast::TryConnectStmt(ast, functionId, ifId), Equals(true));
		});

		it("Can connect with multiple outputs", [&]() {
			ast::Tree ast;

			ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");

			ast::Id ifId    = ast::AddIf({ast, ast::NoId});
			ast::Id call1Id = ast::AddCall({ast, ast::NoId}, functionId);
			ast::Id call2Id = ast::AddCall({ast, ast::NoId}, functionId);

			AssertThat(
			    ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[0], call2Id),
			    Equals(true));
			AssertThat(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[0], Equals(call2Id));

			// Can replace a connection
			AssertThat(
			    ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[0], call1Id),
			    Equals(true));
			AssertThat(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[0], Equals(call1Id));

			// Can connect to a different pin
			AssertThat(
			    ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[1], call2Id),
			    Equals(true));
			AssertThat(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[1], Equals(call2Id));
		});
	});
});
