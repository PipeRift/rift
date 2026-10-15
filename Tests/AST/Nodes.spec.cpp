// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Tree.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Nodes", []()
{
	It("Removes nodes and their children right away", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::Id callId     = ast::AddCall({ast, ast::NoId}, functionId);
		Expect(ast.IsValid(callId)).ToBeTrue();
		Expect(ast.Has<ast::CExprCall>(callId)).ToBeTrue();

		// A deferred removal would keep the component data (and the drawn node) alive until
		// the pools are flushed, so nodes must be removed immediately.
		ast::RemoveNodes(ast, callId);

		Expect(ast.IsValid(callId)).ToBeFalse();
		Expect(ast.Has<ast::CExprCall>(callId)).ToBeFalse();
		Expect(ast.Has<ast::CStmtInput>(callId)).ToBeFalse();
	});
});
