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
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/TypeUtils.h"


namespace Rift::FunctionsSystem
{
	struct CallToSync
	{
		AST::Id id;

		AST::Id functionId;
		CExprInputs* functionInputs;
		CExprOutputs* functionOutputs;

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
			auto& call         = callExprs.Get<CExprCall>(id);
			AST::Id functionId = Types::FindFunctionByName(ast, call.ownerName, call.functionName);
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
		TAccess<CCallDirty, CExprCallId, CExprInputs, CExprOutputs, CIdentifier> access{ast};
		for (AST::Id id : AST::ListAll<CCallDirty, CExprCallId>(callAccess))
		{
			auto& call = callAccess.Get<CExprCallId>(id);
			if (IsNone(call.functionId))
			{
				continue;
			}

			CallToSync cache;
			cache.id              = id;
			cache.functionId      = call.functionId;
			cache.functionOutputs = &access.GetOrAdd<CExprOutputs>(call.functionId);
			cache.functionInputs  = &access.GetOrAdd<CExprInputs>(call.functionId);
			calls.Add(cache);
		}


		// Resolve current call parameters and create missing new pins
		TArray<AST::Id> invalidOutputs;
		for (auto& call : calls)
		{
			auto& callOutputs = access.GetOrAdd<const CExprOutputs>(call.id);

			invalidOutputs.Empty(false);
			invalidOutputs = callOutputs.pinIds;

			for (i32 i = 0; i < call.functionOutputs->pinIds.Size(); ++i)
			{
				const AST::Id pinId = call.functionOutputs->pinIds[i];
				const auto* name    = access.TryGet<const CIdentifier>(id);
				if (!name)
				{
					continue;
				}

				if (i >= callOutputs->pinIds.Size())
				{
					AST::Id id = ast.Create();
					access.Add<CIdentifier>(id, *name);
					callOutputs.AddPin(id);
					continue;
				}

				// Search matching pin to 'pinId' from i to end
				i32 callPinIdx = i;
				while (callPinIdx < callOutputs->pinIds.Size())
				{
					const AST::Id pinId     = callOutputs->pinIds[callPinIdx];
					const auto* callPinName = access.TryGet<const CIdentifier>(pinId);
					if (!callPinName || *callPinName != *name)
						++callPinIdx;
				}
				if (callPinIdx == callOutputs->pinIds.Size())    // Pin not found, insert it
				{
					AST::Id id = ast.Create();
					identifiers.Add<CIdentifier>(id, *name);
					callOutputs.AddPinAt(i, id);
				}
				else if (callPinIdx > i)
				{
					// Correct pin is after where it should, we swap it to ensure correct order
					callOutputs->pinIds.Swap(callPinIdx, i);
				}
			}

			// TODO: Extract last pins exceeding function pin size into invalids
		}

		for (auto& call : calls)
		{
			for (AST::Id id : call.functionInputs->pinIds)
			{
				invalidOutputs.Empty(false);
				const auto& name = identifiers.Get<CIdentifier>(id);

				const i32 idx = invalidOuputs.FindIndex([&identifiers, &name](AST::Id id) {
					return identifiers.Get<CIdentifier>(id) == name;
				});
				if (idx != NO_INDEX)
				{
					call.inputArgs.Add(invalidOutputs[idx]);
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
			// AST::Hierarchy::RemoveAllChildren(ast, call.id, true);
			// AST::Hierarchy::AddChildren(ast, call.id, call.inputArgs);
			// AST::Hierarchy::AddChildren(ast, call.id, call.outputArgs);
			// AST::Hierarchy::AddChildren(ast, call.id, call.invalidArgs);
			// AST::Hierarchy::AddChildren(ast, call.id, call.unrelatedCallChildren);
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
