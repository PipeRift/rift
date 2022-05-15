// Copyright 2015-2022 Piperift - All rights reserved

#include <AST/FilterAccess.h>
#include <AST/Tree.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


go_bandit([]() {
	describe("AST.Expressions", []() {
		it("Initializes inputs & outputs correctly", [&]() {
			AST::Tree ast;

			AST::Id id = Types::AddBinaryOperator({ast, AST::NoId}, BinaryOperatorType::Div);
			AssertThat(ast.Has<CExprOutputs>(id), Equals(true));
			AssertThat(ast.Has<CExprInputs>(id), Equals(true));
			AssertThat(ast.Get<CExprInputs>(id).linkedOutputs.Size(), Equals(2));
			AssertThat(ast.Get<CExprInputs>(id).linkedOutputs.Size(),
			    Equals(ast.Get<CExprInputs>(id).pinIds.Size()));

			AST::Id id2 = Types::AddUnaryOperator({ast, AST::NoId}, UnaryOperatorType::Not);
			AssertThat(ast.Has<CExprOutputs>(id2), Equals(true));
			AssertThat(ast.Has<CExprInputs>(id2), Equals(true));
			AssertThat(ast.Get<CExprInputs>(id2).linkedOutputs.Size(), Equals(1));
			AssertThat(ast.Get<CExprInputs>(id2).linkedOutputs.Size(),
			    Equals(ast.Get<CExprInputs>(id2).pinIds.Size()));
		});
	});
});
