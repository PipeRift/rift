// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Components/Declarations.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Systems/TypeSystem.h>
#include <AST/Tree.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TypeUtils.h>
#include <ASTModule.h>
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

	It("Initializes for loops", []()
	{
		ast::Tree ast;

		ast::Id forId = ast::AddFor({ast, ast::NoId});
		Expect(ast.Has<ast::CStmtFor>(forId)).ToBeTrue();
		Expect(ast.Has<ast::CStmtInput>(forId)).ToBeTrue();

		const auto* inputs = ast.TryGet<const ast::CExprInputs>(forId);
		Expect(inputs != nullptr).ToBeTrue();
		Expect(inputs->pinIds.Size()).ToEqual(2u);
		Expect(inputs->linkedOutputs.Size()).ToEqual(2u);
		Expect(ast.Get<ast::CExprTypeId>(inputs->pinIds[0]).id).ToEqual(ast.GetNativeTypes().i32Id);
		Expect(ast.Get<ast::CExprTypeId>(inputs->pinIds[1]).id).ToEqual(ast.GetNativeTypes().i32Id);
		Expect(ast.Get<ast::CNamespace>(inputs->pinIds[0]).name).ToEqual(p::Tag{"first"});
		Expect(ast.Get<ast::CNamespace>(inputs->pinIds[1]).name).ToEqual(p::Tag{"end"});

		const auto* exprOutputs = ast.TryGet<const ast::CExprOutputs>(forId);
		Expect(exprOutputs != nullptr).ToBeTrue();
		Expect(exprOutputs->pinIds.Size()).ToEqual(1u);
		Expect(ast.Get<ast::CExprTypeId>(exprOutputs->pinIds[0]).id)
		    .ToEqual(ast.GetNativeTypes().i32Id);

		const auto* outputs = ast.TryGet<const ast::CStmtOutputs>(forId);
		Expect(outputs != nullptr).ToBeTrue();
		Expect(outputs->pinIds.Size()).ToEqual(2u);
		Expect(outputs->linkInputNodes.Size()).ToEqual(2u);
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

	It("Connects for loop outputs independently", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::Id forId      = ast::AddFor({ast, ast::NoId});
		ast::Id eachCallId = ast::AddCall({ast, ast::NoId}, functionId);
		ast::Id nextCallId = ast::AddCall({ast, ast::NoId}, functionId);

		const auto& outputs = ast.Get<ast::CStmtOutputs>(forId);
		Expect(outputs.pinIds.Size()).ToEqual(2u);

		// "each" and "continue" accept independent connections
		Expect(ast::TryConnectStmt(ast, outputs.pinIds[0], eachCallId)).ToBeTrue();
		Expect(ast::TryConnectStmt(ast, outputs.pinIds[1], nextCallId)).ToBeTrue();
		Expect(ast.Get<ast::CStmtOutputs>(forId).linkInputNodes[0]).ToEqual(eachCallId);
		Expect(ast.Get<ast::CStmtOutputs>(forId).linkInputNodes[1]).ToEqual(nextCallId);
	});

	It("Disconnects a single output", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::Id callId     = ast::AddCall({ast, ast::NoId}, functionId);

		Expect(ast::TryConnectStmt(ast, functionId, callId)).ToBeTrue();
		Expect(ast.Get<ast::CStmtInput>(callId).linkOutputNode).ToEqual(functionId);

		// Input node id equals the link id
		Expect(ast::DisconnectStmtLink(ast, callId)).ToBeTrue();
		Expect(ast.Get<ast::CStmtInput>(callId).linkOutputNode).ToEqual(ast::NoId);
		Expect(ast.Get<ast::CStmtOutput>(functionId).linkInputNode).ToEqual(ast::NoId);
	});

	It("Invalidates return pins disconnected from their function", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionOutput(ast, functionId, "out", ast.GetNativeTypes().floatId);

		ast::Id returnId = ast::AddReturn({ast, ast::NoId});
		Expect(ast::TryConnectStmt(ast, functionId, returnId)).ToBeTrue();

		ast::FunctionsSystem::SyncReturnPinsFromFunction(ast);
		const auto* inputs = ast.TryGet<const ast::CExprInputs>(returnId);
		Expect(inputs != nullptr).ToBeTrue();
		Expect(inputs->pinIds.Size()).ToEqual(1u);
		const ast::Id pinId = inputs->pinIds[0];
		Expect(ast.IsValid(pinId)).ToBeTrue();

		// Disconnecting the return from its function invalidates the mirrored pins
		Expect(ast::DisconnectStmtLink(ast, returnId)).ToBeTrue();
		ast::FunctionsSystem::SyncReturnPinsFromFunction(ast);
		Expect(ast.IsValid(pinId)).ToBeFalse();
		Expect(ast.Has<ast::CInvalid>(pinId)).ToBeTrue();
	});

	It("Replaces a connected return without corrupting its pins", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::AddFunctionOutput(ast, functionId, "out", ast.GetNativeTypes().floatId);

		ast::Id return1 = ast::AddReturn({ast, ast::NoId});
		ast::Id return2 = ast::AddReturn({ast, ast::NoId});

		Expect(ast::TryConnectStmt(ast, functionId, return1)).ToBeTrue();
		ast::FunctionsSystem::SyncReturnPinsFromFunction(ast);

		// Connecting a second return replaces the first. Creating the new return's pins
		// must not invalidate the reference to the function's component.
		Expect(ast::TryConnectStmt(ast, functionId, return2)).ToBeTrue();
		ast::FunctionsSystem::SyncReturnPinsFromFunction(ast);

		Expect(ast.Get<ast::CStmtInput>(return2).linkOutputNode).ToEqual(functionId);
		Expect(ast.Get<ast::CStmtInput>(return1).linkOutputNode).ToEqual(ast::NoId);
		const auto* inputs = ast.TryGet<const ast::CExprInputs>(return2);
		Expect(inputs != nullptr).ToBeTrue();
		Expect(inputs->pinIds.Size()).ToEqual(1u);
		if (inputs && !inputs->pinIds.IsEmpty())
		{
			Expect(ast.IsValid(inputs->pinIds[0])).ToBeTrue();
		}
	});

	It("Creates assignments with execution and value pins", []()
	{
		ast::Tree ast;

		ast::Id variableId = ast::AddVariable({ast, ast::NoId}, "MyVar");
		ast::Id assignId   = ast::AddAssign({ast, ast::NoId}, variableId);

		Expect(ast.Has<ast::CStmtAssign>(assignId)).ToBeTrue();
		Expect(ast.Get<ast::CStmtAssign>(assignId).declarationId).ToEqual(variableId);

		Expect(ast.Has<ast::CStmtInput>(assignId)).ToBeTrue();
		Expect(ast.Has<ast::CStmtOutput>(assignId)).ToBeTrue();

		const auto* inputs = ast.TryGet<const ast::CExprInputs>(assignId);
		Expect(inputs != nullptr).ToBeTrue();
		Expect(inputs->pinIds.Size()).ToEqual(1u);
		const auto* outputs = ast.TryGet<const ast::CExprOutputs>(assignId);
		Expect(outputs != nullptr).ToBeTrue();
		Expect(outputs->pinIds.Size()).ToEqual(1u);
	});

	It("Connects assignments to other statements", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		ast::Id variableId = ast::AddVariable({ast, ast::NoId}, "MyVar");
		ast::Id assignId   = ast::AddAssign({ast, ast::NoId}, variableId);
		ast::Id callId     = ast::AddCall({ast, ast::NoId}, functionId);

		Expect(ast::TryConnectStmt(ast, functionId, assignId)).ToBeTrue();
		Expect(ast::TryConnectStmt(ast, assignId, callId)).ToBeTrue();

		Expect(ast.Get<ast::CStmtInput>(assignId).linkOutputNode).ToEqual(functionId);
		Expect(ast.Get<ast::CStmtOutput>(assignId).linkInputNode).ToEqual(callId);
	});

	It("Propagates the type of the variable to the assignment pins", []()
	{
		ast::Tree ast;

		ast::Id variableId = ast::AddVariable({ast, ast::NoId}, "MyVar");
		ast.Get<ast::CDeclVariable>(variableId).typeId = ast.GetNativeTypes().i32Id;
		ast::Id assignId = ast::AddAssign({ast, ast::NoId}, variableId);

		ast::TypeSystem::PropagateTypes(ast);

		const auto& inputs       = ast.Get<const ast::CExprInputs>(assignId);
		const auto& outputs      = ast.Get<const ast::CExprOutputs>(assignId);
		const ast::Id valueInId  = inputs.pinIds[0];
		const ast::Id valueOutId = outputs.pinIds[0];

		Expect(ast.Get<ast::CExprTypeId>(valueInId).id).ToEqual(ast.GetNativeTypes().i32Id);
		Expect(ast.Get<ast::CExprTypeId>(valueOutId).id).ToEqual(ast.GetNativeTypes().i32Id);
	});

	It("Does not type the execution pins of an assignment", []()
	{
		ast::Tree ast;

		ast::Id variableId = ast::AddVariable({ast, ast::NoId}, "MyVar");
		ast.Get<ast::CDeclVariable>(variableId).typeId = ast.GetNativeTypes().i32Id;
		ast::Id assignId = ast::AddAssign({ast, ast::NoId}, variableId);

		ast::TypeSystem::PropagateTypes(ast);

		// The node is the execution pin, so it must not be typed as an expression
		Expect(ast.Has<ast::CExprTypeId>(assignId)).ToBeFalse();
		Expect(ast.Has<ast::CExprType>(assignId)).ToBeFalse();
	});

	It("Serializes assignments", []()
	{
		ast::Tree ast;
		const ast::Id typeId     = ast::CreateType(ast, ASTModule::classType, "TestClass");
		const ast::Id variableId = ast::AddVariable({ast, typeId}, "MyVar");
		ast.Get<ast::CDeclVariable>(variableId).typeId = ast.GetNativeTypes().i32Id;
		const ast::Id assignId                         = ast::AddAssign({ast, typeId}, variableId);

		ast::TypeSystem::PropagateTypes(ast);

		p::String data;
		ast::SerializeType(ast, typeId, data);

		// Load the type back into a new tree
		ast::Tree loaded;
		const ast::Id loadedTypeId = ast::CreateType(loaded, ASTModule::classType, "TestClass");
		ast::DeserializeType(loaded, loadedTypeId, data);

		TArray<ast::Id> children;
		p::GetIdChildren(loaded, loadedTypeId, children);
		ast::Id loadedAssignId = ast::NoId;
		for (ast::Id id : children)
		{
			if (loaded.Has<ast::CStmtAssign>(id))
			{
				loadedAssignId = id;
			}
		}
		Expect(loadedAssignId != ast::NoId).ToBeTrue();
		if (loadedAssignId == ast::NoId)
		{
			return;
		}

		const ast::Id loadedVariableId = loaded.Get<ast::CStmtAssign>(loadedAssignId).declarationId;
		Expect(loaded.IsValid(loadedVariableId)).ToBeTrue();
		Expect(loaded.Get<ast::CNamespace>(loadedVariableId).name.AsString().data())
		    .ToEqual("MyVar");
	});
});
