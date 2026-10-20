// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Systems/ConversionSystem.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Systems/TypeSystem.h>
#include <AST/Tree.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Conversions", []()
{
	It("Registers the numeric conversions", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		// Conversions that keep every value of the source type happen on their own
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.i32Id, natives.doubleId))
		    .ToBeTrue();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.floatId, natives.doubleId))
		    .ToBeTrue();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.u8Id, natives.i32Id))
		    .ToBeTrue();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.boolId, natives.i32Id))
		    .ToBeTrue();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.i32Id, natives.i64Id))
		    .ToBeTrue();

		// Narrowing conversions need an explicit cast node
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.doubleId, natives.i32Id))
		    .ToBeFalse();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.doubleId, natives.floatId))
		    .ToBeFalse();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.i32Id, natives.i8Id))
		    .ToBeFalse();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.i32Id, natives.u32Id))
		    .ToBeFalse();
		Expect(ast::ConversionSystem::CanConvertImplicitly(ast, natives.i64Id, natives.doubleId))
		    .ToBeFalse();

		// But they can still be done explicitly
		Expect(ast::ConversionSystem::CanConvertExplicitly(ast, natives.doubleId, natives.i32Id))
		    .ToBeTrue();
		Expect(ast::ConversionSystem::CanConvertExplicitly(ast, natives.i32Id, natives.i8Id))
		    .ToBeTrue();

		// Types that have nothing to do with each other don't convert
		Expect(ast::ConversionSystem::CanConvertExplicitly(ast, natives.stringId, natives.i32Id))
		    .ToBeFalse();
		Expect(ast::ConversionSystem::CanConvertExplicitly(ast, natives.i32Id, natives.stringId))
		    .ToBeFalse();
	});

	It("Connects values that convert into the type of the pin", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "value", natives.doubleId);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		const auto& callInputs = ast.Get<ast::CExprInputs>(callId);
		Expect(callInputs.pinIds.Size()).ToEqual(1u);

		// An I32 value converts into the Double argument
		ast::Id literalId = ast::AddLiteral({ast, ast::NoId}, natives.i32Id);
		Expect(literalId != ast::NoId).ToBeTrue();
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, literalId),
		           ast::GetExprInputFromPin(ast, callInputs.pinIds[0])))
		    .ToBeTrue();
	});

	It("Rejects values that need an explicit conversion", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "value", natives.i32Id);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		const auto& callInputs = ast.Get<ast::CExprInputs>(callId);
		Expect(callInputs.pinIds.Size()).ToEqual(1u);

		// A Double value doesn't fit in the I32 argument
		ast::Id literalId = ast::AddLiteral({ast, ast::NoId}, natives.doubleId);
		Expect(literalId != ast::NoId).ToBeTrue();
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, literalId),
		           ast::GetExprInputFromPin(ast, callInputs.pinIds[0])))
		    .ToBeFalse();
	});

	It("Connects bool values into bool and integer inputs", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "flag", natives.boolId);
		ast::AddFunctionInput(ast, functionId, "value", natives.i64Id);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		const auto& callInputs = ast.Get<ast::CExprInputs>(callId);
		Expect(callInputs.pinIds.Size()).ToEqual(2u);

		ast::Id literalId            = ast::AddLiteral({ast, ast::NoId}, natives.boolId);
		const ast::ExprOutput output = ast::GetExprOutputFromPin(ast, literalId);

		// A bool value fits a bool input
		Expect(
		    ast::TryConnectExpr(ast, output, ast::GetExprInputFromPin(ast, callInputs.pinIds[0])))
		    .ToBeTrue();

		// And converts on its own into an integer one
		Expect(
		    ast::TryConnectExpr(ast, output, ast::GetExprInputFromPin(ast, callInputs.pinIds[1])))
		    .ToBeTrue();

		// The condition of an if takes a bool too
		ast::Id ifId         = ast::AddIf({ast, ast::NoId});
		const auto& ifInputs = ast.Get<ast::CExprInputs>(ifId);
		Expect(ifInputs.pinIds.Size()).ToEqual(1u);
		Expect(ast::TryConnectExpr(ast, output, ast::GetExprInputFromPin(ast, ifInputs.pinIds[0])))
		    .ToBeTrue();
	});

	It("Resolves the types of the pins from their names", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		Id typeId    = ast::CreateType(ast, p::Tag{"TestType"}, "TestType", "");
		Id literalId = ast::AddLiteral({ast, typeId}, natives.boolId);
		Id castId    = ast::AddCast({ast, typeId}, natives.boolId, natives.i64Id);

		// Loading a file doesn't keep the resolved type ids, every pin resolves its type
		// again from the name stored with it
		for (Id id : p::FindAllIdsWith<ast::CExprTypeId>(ast))
		{
			ast.Remove<ast::CExprTypeId>(id);
		}
		ast::TypeSystem::ResolveExprTypeIds(ast);

		const Id inputPinId  = ast.Get<ast::CExprInputs>(castId).pinIds[0];
		const Id outputPinId = ast.Get<ast::CExprOutputs>(castId).pinIds[0];
		Expect(ast::GetPinType(ast, literalId)).ToEqual(natives.boolId);
		Expect(ast::GetPinType(ast, inputPinId)).ToEqual(natives.boolId);
		Expect(ast::GetPinType(ast, outputPinId)).ToEqual(natives.i64Id);

		// And the bool literal goes into the bool input of the cast
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, literalId),
		           ast::GetExprInputFromPin(ast, inputPinId)))
		    .ToBeTrue();
	});

	It("Inserts a cast node when a value only casts explicitly", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TakesInt");
		ast::AddFunctionInput(ast, functionId, "value", natives.i32Id);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		const ast::ExprInput input =
		    ast::GetExprInputFromPin(ast, ast.Get<ast::CExprInputs>(callId).pinIds[0]);
		ast::Id doubleId             = ast::AddLiteral({ast, ast::NoId}, natives.doubleId);
		const ast::ExprOutput output = ast::GetExprOutputFromPin(ast, doubleId);

		// The Double doesn't fit the I32 on its own, so a cast is created in between
		const ast::Id castId = ast::ConnectExprWithCast({ast, ast::NoId}, output, input);
		Expect(castId != ast::NoId).ToBeTrue();
		Expect(ast.Has<ast::CExprCast>(castId)).ToBeTrue();
		Expect(ast.Get<ast::CExprInputs>(castId).linkedOutputs[0].pinId).ToEqual(doubleId);
		Expect(ast.Get<ast::CExprInputs>(callId).linkedOutputs[0].nodeId).ToEqual(castId);

		// A value that fits the pin on its own doesn't need a cast
		ast::Id intId        = ast::AddLiteral({ast, ast::NoId}, natives.i8Id);
		ast::Id secondCallId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
		const ast::ExprInput secondInput =
		    ast::GetExprInputFromPin(ast, ast.Get<ast::CExprInputs>(secondCallId).pinIds[0]);
		Expect(ast::ConnectExprWithCast(
		           {ast, ast::NoId}, ast::GetExprOutputFromPin(ast, intId), secondInput))
		    .ToEqual(ast::NoId);
		Expect(ast.Get<ast::CExprInputs>(secondCallId).linkedOutputs[0].nodeId).ToEqual(intId);
	});

	It("Creates cast nodes with typed pins", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id castId = ast::AddCast({ast, ast::NoId}, natives.doubleId, natives.i32Id);
		Expect(castId != ast::NoId).ToBeTrue();
		Expect(ast.Has<ast::CExprCast>(castId)).ToBeTrue();

		const auto& inputs  = ast.Get<ast::CExprInputs>(castId);
		const auto& outputs = ast.Get<ast::CExprOutputs>(castId);
		Expect(inputs.pinIds.Size()).ToEqual(1u);
		Expect(outputs.pinIds.Size()).ToEqual(1u);
		Expect(ast.Get<ast::CExprTypeId>(inputs.pinIds[0]).id).ToEqual(natives.doubleId);
		Expect(ast.Get<ast::CExprTypeId>(outputs.pinIds[0]).id).ToEqual(natives.i32Id);
	});

	It("Casts values through a cast node", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "value", natives.i32Id);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		ast::Id castId = ast::AddCast({ast, ast::NoId}, natives.doubleId, natives.i32Id);

		const auto& castInputs  = ast.Get<ast::CExprInputs>(castId);
		const auto& castOutputs = ast.Get<ast::CExprOutputs>(castId);
		const auto& callInputs  = ast.Get<ast::CExprInputs>(callId);

		// The Double value goes into the cast, and its result into the argument
		ast::Id literalId = ast::AddLiteral({ast, ast::NoId}, natives.doubleId);
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, literalId),
		           ast::GetExprInputFromPin(ast, castInputs.pinIds[0])))
		    .ToBeTrue();
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, castOutputs.pinIds[0]),
		           ast::GetExprInputFromPin(ast, callInputs.pinIds[0])))
		    .ToBeTrue();
	});
});
