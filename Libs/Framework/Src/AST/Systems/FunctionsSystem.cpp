// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CCallExpr.h"
#include "AST/Components/Tags/CAdded.h"
#include "AST/Utils/FunctionUtils.h"


namespace Rift::FunctionsSystem
{
	void Init(AST::Tree& ast)
	{
		ast.OnAdd<CCallExprId>().Bind([](auto& ast, auto ids) {
			ast.Add<CAdded<CCallExprId>>(ids);
		});
	}

	void ResolveCallFunctionIds(AST::Tree& ast)
	{
		auto callExprs   = ast.Filter<CCallExpr>(AST::TExclude<CCallExprId>());
		auto callExprIds = ast.Filter<CCallExprId>();
		for (AST::Id id : callExprs)
		{
			auto& call = callExprs.Get<CCallExpr>(id);
			AST::Id functionId =
			    AST::Functions::FindFunctionByName(ast, call.typeName, call.functionName);
			if (!IsNone(functionId))
			{
				callExprIds.Add(id, CCallExprId{functionId});
			}
		}
	}

	void SyncCallArguments(AST::Tree& ast)
	{
		auto addedCallExprs = ast.Filter<CAdded<CCallExprId>, CCallExprId>();
		for (AST::Id id : addedCallExprs)
		{
			auto& call = addedCallExprs.Get<CCallExprId>(id);
			// Create arguments
		}
	}

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CAdded<CCallExprId>>().Reset();
	}
}    // namespace Rift::FunctionsSystem
