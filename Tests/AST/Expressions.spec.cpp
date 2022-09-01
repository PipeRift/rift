// Copyright 2015-2022 Piperift - All rights reserved

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
			AST::Tree ast;

			AST::Id id =
			    AST::Types::AddBinaryOperator({ast, AST::NoId}, AST::BinaryOperatorType::Div);
			AssertThat(ast.Has<AST::CExprOutputs>(id), Equals(true));
			AssertThat(ast.Has<AST::CExprInputs>(id), Equals(true));
			AssertThat(ast.Get<AST::CExprInputs>(id).linkedOutputs.Size(), Equals(2));
			AssertThat(ast.Get<AST::CExprInputs>(id).linkedOutputs.Size(),
			    Equals(ast.Get<AST::CExprInputs>(id).pinIds.Size()));

			AST::Id id2 =
			    AST::Types::AddUnaryOperator({ast, AST::NoId}, AST::UnaryOperatorType::Not);
			AssertThat(ast.Has<AST::CExprOutputs>(id2), Equals(true));
			AssertThat(ast.Has<AST::CExprInputs>(id2), Equals(true));
			AssertThat(ast.Get<AST::CExprInputs>(id2).linkedOutputs.Size(), Equals(1));
			AssertThat(ast.Get<AST::CExprInputs>(id2).linkedOutputs.Size(),
			    Equals(ast.Get<AST::CExprInputs>(id2).pinIds.Size()));
		});
	});
});
