// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Functions
{
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
		auto& expr        = ast.Add<CCallExpr>(callId);
		expr.functionId   = functionId;
		expr.typeName     = ast.Get<CType>(typeId).name;
		expr.functionName = ast.Get<CIdentifier>(functionId).name;

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
}    // namespace Rift::AST::Functions
