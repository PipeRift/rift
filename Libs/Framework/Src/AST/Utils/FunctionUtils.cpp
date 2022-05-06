// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprInput.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloating.h"
#include "AST/Components/CLiteralIntegral.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CStmtIf.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/Statements.h"
#include "AST/Utils/TransactionUtils.h"

#include <AST/Components/CStmtReturn.h>


namespace Rift::Functions
{
	AST::Id AddInputArgument(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);

		ast.GetOrAdd<CExprInputs>(functionId).AddPin(id, AST::NoId);
		return id;
	}

	AST::Id AddOutputArgument(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);

		ast.GetOrAdd<CExprOutputs>(functionId).AddPin(id, AST::NoId);
		return id;
	}

	AST::Id AddIf(AST::TypeRef type)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CStmtIf>(id);
		ast.Add<CStmtInput>(id);

		// Bool input
		const AST::Id valueId = ast.Create();
		AST::Hierarchy::AddChildren(ast, id, valueId);
		ast.Add<CExprInput>(id, {valueId, ast.GetNativeTypes().boolId});

		TArray<AST::Id> outIds(2);
		ast.Create(outIds);
		AST::Hierarchy::AddChildren(ast, id, outIds);
		ast.Add<CStmtOutputs>(id, Move(outIds));

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddReturn(AST::TypeRef type)
	{
		AST::Tree& ast         = type.GetAST();
		const AST::Id returnId = ast.Create();
		ast.Add<CStmtReturn>(returnId);
		ast.Add<CStmtInput>(returnId);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), returnId);
		}
		return returnId;
	}

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprOutputs>(id).AddPin(id, literalTypeId);

		bool created        = false;
		const auto& natives = ast.GetNativeTypes();
		// TODO: Refactor this
		if (literalTypeId == natives.boolId)
		{
			ast.Add<CLiteralBool>(id);
			created = true;
		}
		else if (literalTypeId == natives.floatId)
		{
			ast.Add<CLiteralFloating>(id).type = FloatingType::F32;
			created                            = true;
		}
		else if (literalTypeId == natives.doubleId)
		{
			ast.Add<CLiteralFloating>(id).type = FloatingType::F64;
			created                            = true;
		}
		else if (literalTypeId == natives.u8Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U8;
			created                            = true;
		}
		else if (literalTypeId == natives.u16Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U16;
			created                            = true;
		}
		else if (literalTypeId == natives.u32Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U32;
			created                            = true;
		}
		else if (literalTypeId == natives.u64Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U64;
			created                            = true;
		}
		else if (literalTypeId == natives.i8Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S8;
			created                            = true;
		}
		else if (literalTypeId == natives.i16Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S16;
			created                            = true;
		}
		else if (literalTypeId == natives.i32Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S32;
			created                            = true;
		}
		else if (literalTypeId == natives.i64Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S64;
			created                            = true;
		}
		else if (literalTypeId == natives.stringId)
		{
			ast.Add<CLiteralString>(id);
			created = true;
		}

		if (!created)
		{
			ast.Destroy(id);
			return AST::NoId;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddCall(AST::TypeRef type, AST::Id functionId)
	{
		AST::Tree& ast = type.GetAST();

		const AST::Id callId = ast.Create();
		ast.Add<CStmtInput, CStmtOutput>(callId);
		auto& callExprId      = ast.Add<CExprCallId>(callId);
		callExprId.functionId = functionId;

		auto& callExpr        = ast.Add<CExprCall>(callId);
		callExpr.functionName = ast.Get<CIdentifier>(functionId).name;
		const AST::Id typeId  = AST::Hierarchy::GetParent(ast, functionId);
		if (!IsNone(typeId))
		{
			callExpr.ownerName = ast.Get<CType>(typeId).name;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	AST::Id AddDeclarationReference(AST::TypeRef type, AST::Id declId)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).AddPin(id, AST::NoId);    // Types gets resolved by a system later

		const AST::Id typeId = AST::Hierarchy::GetParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CType>(typeId).name;
		declRefExpr.name            = ast.Get<CIdentifier>(declId).name;
		auto& declRefExprId         = ast.Add<CExprDeclRefId>(id);
		declRefExprId.declarationId = declId;

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddUnaryOperator(AST::TypeRef type, UnaryOperatorType operatorType)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprUnaryOperator>(id, {operatorType});
		ast.Add<CExprInput>(id, {id, AST::NoId});
		ast.Add<CExprOutputs>(id).AddPin(id, AST::NoId);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddBinaryOperator(AST::TypeRef type, BinaryOperatorType operatorType)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprBinaryOperator>(id, {operatorType});
		ast.Add<CExprOutputs>(id);

		auto& inputs = ast.Add<CExprInputs>(id);
		inputs.Resize(2);
		ast.Create(inputs.pinIds);
		AST::Hierarchy::AddChildren(ast, id, inputs.pinIds);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id FindFunctionByName(AST::Tree& ast, Name ownerName, Name functionName)
	{
		auto& types = ast.GetStatic<STypes>();
		if (const AST::Id* typeId = types.typesByName.Find(ownerName))
		{
			if (const auto* children = AST::Hierarchy::GetChildren(ast, *typeId))
			{
				auto functions = ast.Filter<CDeclFunction, CIdentifier>();
				for (AST::Id childId : *children)
				{
					if (functions.Has(childId)
					    && functions.Get<CIdentifier>(childId).name == functionName)
					{
						return childId;
					}
				}
			}
		}
		return AST::NoId;
	}

	void RemoveNodes(const RemoveAccess& access, TSpan<AST::Id> ids)
	{
		ScopedChange(access, ids);
		AST::Hierarchy::RemoveDeep(access, ids);
	}

	void GetCallArgs(AST::Tree& ast, TSpan<AST::Id> callIds, TArray<AST::Id>& inputArgIds,
	    TArray<AST::Id>& outputArgIds, TArray<AST::Id>& otherIds)
	{
		auto exprInputs  = ast.Filter<CExprInput>();
		auto exprOutputs = ast.Filter<CExprOutputs>();

		TArray<AST::Id> children;
		for (AST::Id id : callIds)
		{
			AST::Hierarchy::GetChildren(ast, id, children);
		}

		for (AST::Id id : children)
		{
			if (exprInputs.Has(id))
			{
				inputArgIds.Add(id);
			}
			else if (exprOutputs.Has(id))
			{
				outputArgIds.Add(id);
			}
			else
			{
				otherIds.Add(id);
			}
		}
	}
}    // namespace Rift::Functions
