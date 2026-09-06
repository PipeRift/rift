// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Tree.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Statements", []()
{
	It("Initializes outputs correctly", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		Expect(ast.Has<ast::CStmtOutput>(functionId)).ToBeTrue();

		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		Expect(ast.Has<ast::CStmtOutput>(callId)).ToBeTrue();

		ast::Id ifId = ast::AddIf({ast, ast::NoId});
		Expect(ast.Has<ast::CStmtOutputs>(ifId)).ToBeTrue();
		Expect(ast.Get<ast::CStmtOutputs>(ifId).pinIds.Size()).ToEqual(2u);
	});

	It("Initializes inputs correctly", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		Expect(ast.Has<ast::CStmtInput>(functionId)).ToBeFalse();

		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		Expect(ast.Has<ast::CStmtInput>(callId)).ToBeTrue();

		ast::Id ifId = ast::AddIf({ast, ast::NoId});
		Expect(ast.Has<ast::CStmtInput>(ifId)).ToBeTrue();
	});

	It("Can connect with single output", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::Id callId     = ast::AddCall({ast, ast::NoId}, functionId);
		ast::Id ifId       = ast::AddIf({ast, ast::NoId});

		Expect(ast::TryConnectStmt(ast, functionId, callId)).ToBeTrue();

		// Can't connect to self
		Expect(ast::TryConnectStmt(ast, functionId, functionId)).ToBeFalse();

		// Can't connect in loops
		Expect(ast::TryConnectStmt(ast, callId, ifId)).ToBeTrue();
		Expect(ast::TryConnectStmt(ast, ifId, callId)).ToBeFalse();

		// Can replace a connection
		Expect(ast::TryConnectStmt(ast, functionId, ifId)).ToBeTrue();
	});

	It("Can connect with multiple outputs", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");

		ast::Id ifId    = ast::AddIf({ast, ast::NoId});
		ast::Id call1Id = ast::AddCall({ast, ast::NoId}, functionId);
		ast::Id call2Id = ast::AddCall({ast, ast::NoId}, functionId);

		Expect(ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[0], call2Id))
		    .ToBeTrue();
		Expect(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[0]).ToEqual(call2Id);

		// Can replace a connection
		Expect(ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[0], call1Id))
		    .ToBeTrue();
		Expect(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[0]).ToEqual(call1Id);

		// Can connect to a different pin
		Expect(ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(ifId).pinIds[1], call2Id))
		    .ToBeTrue();
		Expect(ast.Get<ast::CStmtOutputs>(ifId).linkInputNodes[1]).ToEqual(call2Id);
	});
});
