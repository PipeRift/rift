// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/FunctionUtils.h"

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::Functions
{
	AST::Id GetCompoundStmt(AST::TypeRef type, AST::Id functionId)
	{
		return AST::NoId;
	}

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId)
	{
		AST::Tree& ast          = type.GetAST();
		const AST::Id literalId = ast.Create();

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
			return AST::NoId;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), literalId);
		}
		return literalId;
	}

	AST::Id AddCall(AST::TypeRef type, AST::Id targetFunctionId)
	{
		AST::Tree& ast       = type.GetAST();
		const AST::Id callId = ast.Create();

		ast.Add<CStatementInput, CStatementOutputs>(callId);

		auto& expr      = ast.Add<CCallExpr>(callId);
		expr.functionId = targetFunctionId;

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	AST::Id FindStmtType(const AST::Tree& ast, AST::Id stmt)
	{
		AST::Id parentId;
		while (auto* cChild = ast.GetChildView().TryGet<CChild>(parentId))
		{
			parentId = cChild->parent;
			if (ast.Has<CType>(parentId))
			{
				return parentId;
			}
		}
		return AST::NoId;
	}
}    // namespace Rift::Functions
