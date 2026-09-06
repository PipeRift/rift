// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Tree.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Expressions", []()
{
	It("Initializes inputs & outputs correctly", []()
	{
		ast::Tree ast;

		ast::Id id = ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Div);
		Expect(ast.Has<ast::CExprOutputs>(id)).ToBeTrue();
		Expect(ast.Has<ast::CExprInputs>(id)).ToBeTrue();
		Expect(ast.Get<ast::CExprInputs>(id).linkedOutputs.Size()).ToEqual(2u);
		Expect(ast.Get<ast::CExprInputs>(id).linkedOutputs.Size())
		    .ToEqual(ast.Get<ast::CExprInputs>(id).pinIds.Size());

		ast::Id id2 = ast::AddUnaryOperator({ast, ast::NoId}, ast::UnaryOperatorType::Not);
		Expect(ast.Has<ast::CExprOutputs>(id2)).ToBeTrue();
		Expect(ast.Has<ast::CExprInputs>(id2)).ToBeTrue();
		Expect(ast.Get<ast::CExprInputs>(id2).linkedOutputs.Size()).ToEqual(1u);
		Expect(ast.Get<ast::CExprInputs>(id2).linkedOutputs.Size())
		    .ToEqual(ast.Get<ast::CExprInputs>(id2).pinIds.Size());
	});
});
