// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CDeclParameter.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprInput.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloat.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CStmtIf.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/StatementGraph.h"

#include <AST/Components/CExprReturn.h>



namespace Rift::AST::Functions
{
	Id AddInputArgument(Tree& ast, Id functionId)
	{
		Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		ast.Add<CDeclParameter>(id);
		ast.Add<CExprOutputs>(id);

		Hierarchy::AddChildren(ast, functionId, id);
		return id;
	}

	Id AddOutputArgument(Tree& ast, Id functionId)
	{
		Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		ast.Add<CDeclParameter>(id);
		ast.Add<CExprInput>(id);

		Hierarchy::AddChildren(ast, functionId, id);
		return id;
	}

	Id AddIf(TypeRef type)
	{
		Tree& ast   = type.GetAST();
		const Id id = ast.Create();
		ast.Add<CStmtIf>(id);
		ast.Add<CStmtInput>(id);
		ast.Add<CStmtOutputs>(id);

		// Bool input
		const Id valueId = ast.Create();
		ast.Add<CExprInput>(valueId);
		Hierarchy::AddChildren(ast, id, valueId);

		const Id trueId  = ast.Create();
		const Id falseId = ast.Create();
		Hierarchy::AddChildren(ast, id, trueId);
		Hierarchy::AddChildren(ast, id, falseId);

		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddReturn(TypeRef type)
	{
		Tree& ast         = type.GetAST();
		const Id returnId = ast.Create();
		ast.Add<CExprReturn>(returnId);
		ast.Add<CStmtInput>(returnId);
		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), returnId);
		}
		return returnId;
	}

	Id AddLiteral(TypeRef type, Id literalTypeId)
	{
		Tree& ast          = type.GetAST();
		const Id literalId = ast.Create();

		bool created        = false;
		const auto& natives = ast.GetNativeTypes();
		if (literalTypeId == natives.boolId)
		{
			ast.Add<CLiteralBool>(literalId);
			created = true;
		}
		else if (literalTypeId == natives.floatId)
		{
			ast.Add<CLiteralFloat>(literalId);
			created = true;
		}
		else if (literalTypeId == natives.stringId)
		{
			ast.Add<CLiteralString>(literalId);
			created = true;
		}

		if (!created)
		{
			ast.Destroy(literalId);
			return NoId;
		}

		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), literalId);
		}
		return literalId;
	}

	Id AddCall(TypeRef type, Id functionId)
	{
		Tree& ast       = type.GetAST();
		const Id callId = ast.Create();

		ast.Add<CStmtInput, CStmtOutputs>(callId);

		const Id typeId = Hierarchy::GetParent(ast, functionId);
		Check(!IsNone(typeId));
		auto& callExpr        = ast.Add<CExprCall>(callId);
		callExpr.ownerName    = ast.Get<CType>(typeId).name;
		callExpr.functionName = ast.Get<CIdentifier>(functionId).name;
		auto& callExprId      = ast.Add<CExprCallId>(callId);
		callExprId.functionId = functionId;

		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	Id AddDeclarationReference(TypeRef type, Id declId)
	{
		Tree& ast   = type.GetAST();
		const Id id = ast.Create();

		ast.Add<CExprDeclRef, CExprOutputs>(id);

		const Id typeId = Hierarchy::GetParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CType>(typeId).name;
		declRefExpr.name            = ast.Get<CIdentifier>(declId).name;
		auto& declRefExprId         = ast.Add<CExprDeclRefId>(id);
		declRefExprId.declarationId = declId;

		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id FindFunctionByName(Tree& ast, Name ownerName, Name functionName)
	{
		auto& types = ast.GetStatic<STypes>();
		if (const Id* typeId = types.typesByName.Find(ownerName))
		{
			if (const auto* children = Hierarchy::GetChildren(ast, *typeId))
			{
				auto functions = ast.Filter<CDeclFunction, CIdentifier>();
				for (Id childId : *children)
				{
					if (functions.Has(childId)
					    && functions.Get<CIdentifier>(childId).name == functionName)
					{
						return childId;
					}
				}
			}
		}
		return NoId;
	}

	void GetCallArgs(Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
	    TArray<Id>& outputArgIds, TArray<Id>& otherIds)
	{
		auto exprInputs  = ast.Filter<CExprInput>();
		auto exprOutputs = ast.Filter<CExprOutputs>();

		TArray<Id> children;
		for (Id id : callIds)
		{
			Hierarchy::GetChildren(ast, id, children);
		}

		for (Id id : children)
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
}    // namespace Rift::AST::Functions
