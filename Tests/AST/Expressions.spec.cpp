// Copyright 2015-2023 Piperift - All rights reserved

#include <AST/Tree.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace rift;


go_bandit([]() {
	describe("AST.Expressions", []() {
		it("Initializes inputs & outputs correctly", [&]() {
			ast::Tree ast;

			ast::Id id = ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Div);
			AssertThat(ast.Has<ast::CExprOutputs>(id), Equals(true));
			AssertThat(ast.Has<ast::CExprInputs>(id), Equals(true));
			AssertThat(ast.Get<ast::CExprInputs>(id).linkedOutputs.Size(), Equals(2));
			AssertThat(ast.Get<ast::CExprInputs>(id).linkedOutputs.Size(),
			    Equals(ast.Get<ast::CExprInputs>(id).pinIds.Size()));

			ast::Id id2 = ast::AddUnaryOperator({ast, ast::NoId}, ast::UnaryOperatorType::Not);
			AssertThat(ast.Has<ast::CExprOutputs>(id2), Equals(true));
			AssertThat(ast.Has<ast::CExprInputs>(id2), Equals(true));
			AssertThat(ast.Get<ast::CExprInputs>(id2).linkedOutputs.Size(), Equals(1));
			AssertThat(ast.Get<ast::CExprInputs>(id2).linkedOutputs.Size(),
			    Equals(ast.Get<ast::CExprInputs>(id2).pinIds.Size()));
		});
	});
});
