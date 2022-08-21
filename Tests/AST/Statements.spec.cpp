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

			AST::Id functionId = Types::AddFunction({ast, AST::NoId}, "TestFunction");
			AssertThat(ast.Has<CStmtOutput>(functionId), Equals(true));

			AST::Id callId = Types::AddCall({ast, AST::NoId}, functionId);
			AssertThat(ast.Has<CStmtOutput>(callId), Equals(true));

			AST::Id ifId = Types::AddIf({ast, AST::NoId});
			AssertThat(ast.Has<CStmtOutputs>(ifId), Equals(true));
			AssertThat(ast.Get<CStmtOutputs>(ifId).pinIds.Size(), Equals(2));
		});

		it("Initializes inputs correctly", [&]() {
			AST::Tree ast;

			AST::Id functionId = Types::AddFunction({ast, AST::NoId}, "TestFunction");
			AssertThat(ast.Has<CStmtInput>(functionId), Equals(false));

			AST::Id callId = Types::AddCall({ast, AST::NoId}, functionId);
			AssertThat(ast.Has<CStmtInput>(callId), Equals(true));

			AST::Id ifId = Types::AddIf({ast, AST::NoId});
			AssertThat(ast.Has<CStmtInput>(ifId), Equals(true));
		});

		it("Can connect with single output", [&]() {
			AST::Tree ast;

			AST::Id functionId = Types::AddFunction({ast, AST::NoId}, "TestFunction");
			AST::Id callId     = Types::AddCall({ast, AST::NoId}, functionId);
			AST::Id ifId       = Types::AddIf({ast, AST::NoId});

			AssertThat(AST::Statements::TryConnect(ast, functionId, callId), Equals(true));

			// Can't connect to self
			AssertThat(AST::Statements::TryConnect(ast, functionId, functionId), Equals(false));

			// Can't connect in loops
			AssertThat(AST::Statements::TryConnect(ast, callId, ifId), Equals(true));
			AssertThat(AST::Statements::TryConnect(ast, ifId, callId), Equals(false));

			// Can replace a connection
			AssertThat(AST::Statements::TryConnect(ast, functionId, ifId), Equals(true));
		});

		it("Can connect with multiple outputs", [&]() {
			AST::Tree ast;

			AST::Id functionId = Types::AddFunction({ast, AST::NoId}, "TestFunction");

			AST::Id ifId    = Types::AddIf({ast, AST::NoId});
			AST::Id call1Id = Types::AddCall({ast, AST::NoId}, functionId);
			AST::Id call2Id = Types::AddCall({ast, AST::NoId}, functionId);

			AssertThat(
			    AST::Statements::TryConnect(ast, ast.Get<CStmtOutputs>(ifId).pinIds[0], call2Id),
			    Equals(true));
			AssertThat(ast.Get<CStmtOutputs>(ifId).linkInputNodes[0], Equals(call2Id));

			// Can replace a connection
			AssertThat(
			    AST::Statements::TryConnect(ast, ast.Get<CStmtOutputs>(ifId).pinIds[0], call1Id),
			    Equals(true));
			AssertThat(ast.Get<CStmtOutputs>(ifId).linkInputNodes[0], Equals(call1Id));

			// Can connect to a different pin
			AssertThat(
			    AST::Statements::TryConnect(ast, ast.Get<CStmtOutputs>(ifId).pinIds[1], call2Id),
			    Equals(true));
			AssertThat(ast.Get<CStmtOutputs>(ifId).linkInputNodes[1], Equals(call2Id));
		});
	});
});