// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CCallExpr.h"
#include "AST/Components/CExpressionInput.h"
#include "AST/Components/CExpressionOutputs.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParameterDecl.h"
#include "AST/Components/Tags/CAdded.h"
#include "AST/Utils/FunctionUtils.h"
#include "AST/Utils/Hierarchy.h"

namespace Rift::FunctionsSystem
{
	struct CallToSync
	{
		AST::Id id;

		AST::Id functionId;
		TArray<AST::Id> functionInputs;
		TArray<AST::Id> functionOutputs;

		TArray<AST::Id> outputArgs;
		TArray<AST::Id> inputArgs;
	};


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
		auto functionParams = ast.Filter<CParameterDecl, CExpressionInput, CExpressionOutputs>();
		auto identifiers    = ast.Filter<CIdentifier>();
		auto exprInputs     = ast.Filter<CExpressionInput>();
		auto exprOutputs    = ast.Filter<CExpressionOutputs>();

		TArray<CallToSync> calls;

		auto addedCallExprs = ast.Filter<CAdded<CCallExprId>, CCallExprId>();
		for (AST::Id id : addedCallExprs)
		{
			auto& call = addedCallExprs.Get<CCallExprId>(id);
			if (IsNone(call.functionId))
			{
				continue;
			}

			CallToSync cache;
			cache.id         = id;
			cache.functionId = call.functionId;
			calls.Add(cache);
		}

		// Resolve function parameters
		for (auto& call : calls)
		{
			if (auto* functionChildren = AST::Hierarchy::GetChildren(ast, call.functionId))
			{
				// Find function inputs and outputs
				call.functionInputs.Resize(functionChildren->Size());
				call.functionOutputs.Resize(functionChildren->Size());
				for (AST::Id childId : *functionChildren)
				{
					if (functionParams.Has<CParameterDecl>(childId))
					{
						if (functionParams.Has<CExpressionOutputs>(childId))
						{
							call.functionInputs.Add(childId);
						}
						else if (functionParams.Has<CExpressionInput>(childId))
						{
							call.functionOutputs.Add(childId);
						}
					}
				}
				identifiers.FilterIds(call.functionInputs);
				identifiers.FilterIds(call.functionOutputs);
			}
		}

		// Resolve current call parameters
		for (auto& call : calls)
		{
			AST::Functions::GetCallArgs(ast, call.id, call.inputArgs, call.outputArgs);
		}

		// Update call parameters. Keep invalid & connected but mark as such
		for (auto& call : calls)
		{
			TArray<AST::Id> invalidArgs;

			for (AST::Id id : call.inputArgs)
			{
				const CIdentifier& name = identifiers.Get<CIdentifier>(id);
				const bool contained =
				    call.functionOutputs.Contains([&identifiers, &name](AST::Id id) {
					    return identifiers.Get<CIdentifier>(id) == name;
				    });
				if (!contained)
				{
					invalidArgs.Add(id);
				}
			}

			for (AST::Id id : call.outputArgs)
			{
				const CIdentifier& name = identifiers.Get<CIdentifier>(id);
				const bool contained =
				    call.functionInputs.Contains([&identifiers, &name](AST::Id id) {
					    return identifiers.Get<CIdentifier>(id) == name;
				    });
				if (!contained)
				{
					invalidArgs.Add(id);
				}
			}
		}
	}

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CAdded<CCallExprId>>().Reset();
	}
}    // namespace Rift::FunctionsSystem
