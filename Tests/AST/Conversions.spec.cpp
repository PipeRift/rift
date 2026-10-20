// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Systems/ConversionSystem.h>
#include <AST/Systems/FunctionsSystem.h>
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

		// Narrowing conversions need an explicit conversion node
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

	It("Creates conversion nodes with typed pins", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id conversionId =
		    ast::AddConversion({ast, ast::NoId}, natives.doubleId, natives.i32Id);
		Expect(conversionId != ast::NoId).ToBeTrue();
		Expect(ast.Has<ast::CExprConversion>(conversionId)).ToBeTrue();

		const auto& inputs  = ast.Get<ast::CExprInputs>(conversionId);
		const auto& outputs = ast.Get<ast::CExprOutputs>(conversionId);
		Expect(inputs.pinIds.Size()).ToEqual(1u);
		Expect(outputs.pinIds.Size()).ToEqual(1u);
		Expect(ast.Get<ast::CExprTypeId>(inputs.pinIds[0]).id).ToEqual(natives.doubleId);
		Expect(ast.Get<ast::CExprTypeId>(outputs.pinIds[0]).id).ToEqual(natives.i32Id);
	});

	It("Converts values through a conversion node", []()
	{
		ast::Tree ast;
		const auto& natives = ast.GetNativeTypes();

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionInput(ast, functionId, "value", natives.i32Id);
		ast::Id callId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::FunctionsSystem::SyncCallPinsFromFunction(ast);

		ast::Id conversionId =
		    ast::AddConversion({ast, ast::NoId}, natives.doubleId, natives.i32Id);

		const auto& conversionInputs  = ast.Get<ast::CExprInputs>(conversionId);
		const auto& conversionOutputs = ast.Get<ast::CExprOutputs>(conversionId);
		const auto& callInputs        = ast.Get<ast::CExprInputs>(callId);

		// The Double value goes into the conversion, and its result into the argument
		ast::Id literalId = ast::AddLiteral({ast, ast::NoId}, natives.doubleId);
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, literalId),
		           ast::GetExprInputFromPin(ast, conversionInputs.pinIds[0])))
		    .ToBeTrue();
		Expect(ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, conversionOutputs.pinIds[0]),
		           ast::GetExprInputFromPin(ast, callInputs.pinIds[0])))
		    .ToBeTrue();
	});
});
