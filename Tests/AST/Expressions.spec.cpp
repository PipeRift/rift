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
