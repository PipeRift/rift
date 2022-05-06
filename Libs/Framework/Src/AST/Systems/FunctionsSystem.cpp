// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CIdentifier.h"
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
		ast.OnAdd<CExprCallId>().Bind([](auto& ast, auto ids) {
			ast.template Add<CCallDirty>(ids);
		});
	}

	void ResolveCallFunctionIds(AST::Tree& ast)
	{
		auto callExprs   = ast.Filter<CExprCall>(AST::TExclude<CExprCallId>());
		auto callExprIds = ast.Filter<CExprCallId>();
		for (AST::Id id : callExprs)
		{
			auto& call = callExprs.Get<CExprCall>(id);
			AST::Id functionId =
			    Functions::FindFunctionByName(ast, call.ownerName, call.functionName);
			if (!IsNone(functionId))
			{
				callExprIds.Add(id, CExprCallId{functionId});
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

		auto callExprIds = ast.Filter<CExprCallId>(AST::TExclude<CCallDirty>{});
		auto dirtyCalls  = ast.Filter<CCallDirty>();
		for (AST::Id id : callExprIds)
		{
			const AST::Id functionId = callExprIds.Get<CExprCallId>(id).functionId;
			if (!IsNone(functionId) && changed.Has(functionId))
			{
				dirtyCalls.Add<CCallDirty>(id);
			}
		}
	}

	void SyncCallArguments(AST::Tree& ast)
	{
		auto functionParams = ast.Filter<CExprType, CExprInput, CExprOutputs>();
		auto identifiers    = ast.Filter<CIdentifier>();
		auto exprInputs     = ast.Filter<CExprInput>();
		auto exprOutputs    = ast.Filter<CExprOutputs>();

		TArray<CallToSync> calls;
		auto dirtyCallExprs = ast.Filter<CCallDirty, CExprCallId>();
		for (AST::Id id : dirtyCallExprs)
		{
			auto& call = dirtyCallExprs.Get<CExprCallId>(id);
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
			if (const auto* functionChildren = AST::Hierarchy::GetChildren(ast, call.functionId))
			{
				// Find function inputs and outputs
				call.functionInputs.Resize(functionChildren->Size());
				call.functionOutputs.Resize(functionChildren->Size());
				for (AST::Id childId : *functionChildren)
				{
					if (functionParams.Has<CExprType>(childId))
					{
						if (functionParams.Has<CExprOutputs>(childId))
						{
							call.functionInputs.Add(childId);
						}
						else if (functionParams.Has<CExprInput>(childId))
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
			Functions::GetCallArgs(
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
					exprOutputs.Add<CExprOutputs>(id);
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
					exprInputs.Add<CExprInput>(id);
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
		auto invalidInputs  = ast.Filter<CInvalid, CExprInput>();
		auto invalidOutputs = ast.Filter<CInvalid, CExprOutputs>();
		TArray<AST::Id> disconnectedPins;
		for (AST::Id id : invalidInputs)
		{
			auto& input = invalidInputs.Get<CExprInput>(id);
			if (IsNone(input.linkOutputPin))
			{
				disconnectedPins.Add(id);
			}
		}
		for (AST::Id id : invalidOutputs)
		{
			auto& output = invalidOutputs.Get<CExprOutputs>(id);
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
