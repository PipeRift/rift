// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CCallExpr.h"
#include "AST/Components/Tags/CAdded.h"


namespace Rift::FunctionsSystem
{
	void Init(AST::Tree& ast)
	{
		ast.OnAdd<CCallExpr>().Bind([](auto& ast, auto ids) {
			for (AST::Id id : ids)
			{
				ast.Add<CAdded<CCallExpr>>(id);
			}
			// TODO: Add range
		});
	}

	void SyncCallArguments(AST::Tree& ast)
	{
		auto callExprs = ast.Filter<CAdded<CCallExpr>, CCallExpr>();
		for (AST::Id id : callExprs)
		{
			auto& expr = callExprs.Get<CCallExpr>(id);
			expr.typeName;
			// Create arguments
		}
	}

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CAdded<CCallExpr>>().Reset();
	}
}    // namespace Rift::FunctionsSystem
