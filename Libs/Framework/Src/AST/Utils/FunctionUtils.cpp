// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CExpressionInput.h"
#include "AST/Components/CExpressionOutputs.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParameterDecl.h"
#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Functions
{
	Id AddInputArgument(Tree& ast, Id functionId)
	{
		Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		ast.Add<CParameterDecl>(id);
		ast.Add<CExpressionOutputs>(id);

		Hierarchy::AddChildren(ast, functionId, id);
		return id;
	}

	Id AddOutputArgument(Tree& ast, Id functionId)
	{
		Id id = ast.Create();
		ast.Add<CIdentifier>(id);
		ast.Add<CParameterDecl>(id);
		ast.Add<CExpressionInput>(id);

		Hierarchy::AddChildren(ast, functionId, id);
		return id;
	}

	Id AddLiteral(TypeRef type, Id literalTypeId)
	{
		Tree& ast          = type.GetAST();
		const Id literalId = ast.Create();

		bool created        = false;
		const auto& natives = ast.GetNativeTypes();
		if (literalTypeId == natives.boolId)
		{
			ast.Add<CBoolLiteral>(literalId);
			created = true;
		}
		else if (literalTypeId == natives.floatId)
		{
			ast.Add<CFloatLiteral>(literalId);
			created = true;
		}
		else if (literalTypeId == natives.stringId)
		{
			ast.Add<CStringLiteral>(literalId);
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

		ast.Add<CStatementInput, CStatementOutputs>(callId);

		const Id typeId = Hierarchy::GetParent(ast, functionId);
		Check(!IsNone(typeId));
		auto& callExpr        = ast.Add<CCallExpr>(callId);
		callExpr.typeName     = ast.Get<CType>(typeId).name;
		callExpr.functionName = ast.Get<CIdentifier>(functionId).name;
		auto& callExprId      = ast.Add<CCallExprId>(callId);
		callExprId.functionId = functionId;

		if (type)
		{
			Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	Id FindFunctionByName(const Tree& ast, Name typeName, Name functionName)
	{
		auto& types = ast.GetStatic<STypes>();
		if (const Id* typeId = types.typesByName.Find(typeName))
		{
			if (const auto* children = Hierarchy::GetChildren(ast, *typeId))
			{
				auto functions = ast.Filter<CFunctionDecl, CIdentifier>();
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

	void GetCallArgs(const Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
	    TArray<Id>& outputArgIds, TArray<Id>& otherIds)
	{
		auto exprInputs  = ast.Filter<CExpressionInput>();
		auto exprOutputs = ast.Filter<CExpressionOutputs>();

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
