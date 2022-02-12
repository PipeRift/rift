// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExpressionInput.h"
#include "AST/Components/CExpressionOutputs.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloat.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CParameterDecl.h"
#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
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

		ast.Add<CStatementInput, CStatementOutputs>(callId);

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

		ast.Add<CExprDeclRef, CExpressionOutputs>(id);

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

	void GetCallArgs(Tree& ast, TSpan<Id> callIds, TArray<Id>& inputArgIds,
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
