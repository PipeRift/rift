// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CCallExpr.h"
#include "AST/Components/CExpressionInput.h"
#include "AST/Components/CExpressionOutputs.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParameterDecl.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Components/Tags/CInvalid.h"
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

		TArray<AST::Id> inputArgs;
		TArray<AST::Id> outputArgs;
		TArray<AST::Id> invalidArgs;
		TArray<AST::Id> unrelatedCallChildren;
	};


	void Init(AST::Tree& ast)
	{
		ast.OnAdd<CCallExprId>().Bind([](auto& ast, auto ids) {
			ast.template Add<CCallDirty>(ids);
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
			    AST::Functions::FindFunctionByName(ast, call.ownerName, call.functionName);
			if (!IsNone(functionId))
			{
				callExprIds.Add(id, CCallExprId{functionId});
			}
		}
	}

	void PropagateDirtyIntoCalls(AST::Tree& ast)
	{
		auto changed = ast.Filter<CChanged>();
		if (changed.Size() <= 0)
		{
			return;
		}

		auto callExprIds = ast.Filter<CCallExprId>(AST::TExclude<CCallDirty>{});
		auto dirtyCalls  = ast.Filter<CCallDirty>();
		for (AST::Id id : callExprIds)
		{
			const AST::Id functionId = callExprIds.Get<CCallExprId>(id).functionId;
			if (!IsNone(functionId) && changed.Has(functionId))
			{
				dirtyCalls.Add<CCallDirty>(id);
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
		auto dirtyCallExprs = ast.Filter<CCallDirty, CCallExprId>();
		for (AST::Id id : dirtyCallExprs)
		{
			auto& call = dirtyCallExprs.Get<CCallExprId>(id);
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
				identifiers.FilterIdsStable(call.functionInputs);
				identifiers.FilterIdsStable(call.functionOutputs);
			}
		}

		// Resolve current call parameters and create missing new pins
		TArray<AST::Id> currentInputs, currentOutputs;
		for (auto& call : calls)
		{
			currentInputs.Empty(false);
			currentOutputs.Empty(false);
			AST::Functions::GetCallArgs(
			    ast, call.id, currentInputs, currentOutputs, call.unrelatedCallChildren);

			for (AST::Id id : call.functionOutputs)
			{
				const auto& name = identifiers.Get<CIdentifier>(id);

				const i32 idx = currentOutputs.FindIndex([&identifiers, &name](AST::Id id) {
					return identifiers.Get<CIdentifier>(id) == name;
				});
				if (idx != NO_INDEX)
				{
					call.outputArgs.Add(currentOutputs[idx]);
					currentOutputs.RemoveAtSwap(idx, false);
				}
				else
				{
					AST::Id id = ast.Create();
					identifiers.Add<CIdentifier>(id, name);
					exprOutputs.Add<CExpressionOutputs>(id);
					call.outputArgs.Add(id);
				}
			}

			for (AST::Id id : call.functionInputs)
			{
				const auto& name = identifiers.Get<CIdentifier>(id);

				const i32 idx = currentInputs.FindIndex([&identifiers, &name](AST::Id id) {
					return identifiers.Get<CIdentifier>(id) == name;
				});
				if (idx != NO_INDEX)
				{
					call.inputArgs.Add(currentInputs[idx]);
					currentInputs.RemoveAtSwap(idx, false);
				}
				else
				{
					AST::Id id = ast.Create();
					identifiers.Add<CIdentifier>(id, name);
					exprInputs.Add<CExpressionInput>(id);
					call.inputArgs.Add(id);
				}
			}

			call.invalidArgs.Append(currentInputs);
			call.invalidArgs.Append(currentOutputs);
		}

		// Mark or delete invalid args
		for (auto& call : calls)
		{
			ast.Add<CInvalid>(call.invalidArgs);
		}

		for (auto& call : calls)
		{
			AST::Hierarchy::RemoveAllChildren(ast, call.id, true);
			AST::Hierarchy::AddChildren(ast, call.id, call.inputArgs);
			AST::Hierarchy::AddChildren(ast, call.id, call.outputArgs);
			AST::Hierarchy::AddChildren(ast, call.id, call.invalidArgs);
			AST::Hierarchy::AddChildren(ast, call.id, call.unrelatedCallChildren);
		}

		RemoveInvalidDisconnectedArgs(ast);
	}

	void RemoveInvalidDisconnectedArgs(AST::Tree& ast)
	{
		auto invalidInputs  = ast.Filter<CInvalid, CExpressionInput>();
		auto invalidOutputs = ast.Filter<CInvalid, CExpressionOutputs>();
		TArray<AST::Id> disconnectedPins;
		for (AST::Id id : invalidInputs)
		{
			auto& input = invalidInputs.Get<CExpressionInput>(id);
			if (IsNone(input.linkOutputPin))
			{
				disconnectedPins.Add(id);
			}
		}
		for (AST::Id id : invalidOutputs)
		{
			auto& output = invalidOutputs.Get<CExpressionOutputs>(id);
			if (output.linkInputPins.IsEmpty())
			{
				disconnectedPins.Add(id);
			}
		}
		AST::Hierarchy::Remove(ast, disconnectedPins);
	}

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CCallDirty>().Reset();
	}
}    // namespace Rift::FunctionsSystem