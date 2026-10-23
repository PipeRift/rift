// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Systems/FunctionsSystem.h>
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

	It("Connects and disconnects expression pins", []()
	{
		ast::Tree ast;

		const ast::Id add = ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Add);
		const ast::Id mul = ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Mul);
		const ast::Id div = ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Div);

		// An operator outputs through its own id, and inputs through its child pin ids
		const auto outputOf = [](ast::Id nodeId)
		{
			ast::ExprOutput output;
			output.nodeId = nodeId;
			output.pinId  = nodeId;
			return output;
		};
		const auto inputOf = [&ast](ast::Id nodeId, i32 index)
		{
			ast::ExprInput input;
			input.nodeId = nodeId;
			input.pinId  = ast.Get<ast::CExprInputs>(nodeId).pinIds[index];
			return input;
		};

		// mul feeds both inputs of add, and add feeds div
		Expect(ast::TryConnectExpr(ast, outputOf(mul), inputOf(add, 0))).ToBeTrue();
		Expect(ast::TryConnectExpr(ast, outputOf(mul), inputOf(add, 1))).ToBeTrue();
		Expect(ast::TryConnectExpr(ast, outputOf(add), inputOf(div, 0))).ToBeTrue();

		// div feeds back into mul through a shared subexpression, so the link would loop
		Expect(ast::CanConnectExpr(ast, outputOf(div), inputOf(mul, 0))).ToBeFalse();
		Expect(ast::TryConnectExpr(ast, outputOf(div), inputOf(mul, 0))).ToBeFalse();

		// Once add no longer reaches mul, the same link is valid
		Expect(ast::DisconnectExpr(ast, inputOf(add, 0))).ToBeTrue();
		Expect(ast::CanConnectExpr(ast, outputOf(div), inputOf(mul, 0))).ToBeFalse();
		Expect(ast::DisconnectExpr(ast, inputOf(add, 1))).ToBeTrue();
		Expect(ast::CanConnectExpr(ast, outputOf(div), inputOf(mul, 0))).ToBeTrue();
		Expect(ast::TryConnectExpr(ast, outputOf(div), inputOf(mul, 0))).ToBeTrue();
	});

	It("Removes pins by marking them as invalid", []()
	{
		ast::Tree ast;

		const ast::Id node =
		    ast::AddBinaryOperator({ast, ast::NoId}, ast::BinaryOperatorType::Add);

		// An operator's output pin is the node itself
		ast::ExprOutput output;
		output.nodeId = node;
		output.pinId  = node;
		Expect(ast::RemoveExprOutputPin(ast, output)).ToBeTrue();
		Expect(ast.Has<ast::CInvalid>(node)).ToBeTrue();

		// Removing twice is harmless, and an empty reference removes nothing
		Expect(ast::RemoveExprOutputPin(ast, output)).ToBeTrue();
		Expect(ast::RemoveExprOutputPin(ast, ast::ExprOutput{})).ToBeFalse();
		Expect(ast::RemoveExprInputPin(ast, ast::ExprInput{})).ToBeFalse();
	});

	It("Syncs the pins of a newly placed call", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "value", ast.GetNativeTypes().floatId);
		ast::AddFunctionOutput(ast, functionId, "out", ast.GetNativeTypes().floatId);

		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);

		// Adding a call has to tag it as modified, otherwise the systems never populate its
		// pins (std functions are never edited, so nothing else marks their calls dirty)
		Expect(ast.Has<p::CMdfd<ast::CExprCallId>>(callId)).ToBeTrue();

		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		// Call inputs mirror the function parameters, outputs its return values
		const auto* callInputs  = ast.TryGet<const ast::CExprInputs>(callId);
		const auto* callOutputs = ast.TryGet<const ast::CExprOutputs>(callId);
		Expect(callInputs != nullptr).ToBeTrue();
		Expect(callInputs->pinIds.Size()).ToEqual(1u);
		Expect(callOutputs != nullptr).ToBeTrue();
		Expect(callOutputs->pinIds.Size()).ToEqual(1u);
	});
});
