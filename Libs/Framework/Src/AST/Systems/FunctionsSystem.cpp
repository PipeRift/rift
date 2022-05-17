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
#include "AST/Filtering.h"
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
		TAccess<CChanged, CExprCallId, TWrite<CCallDirty>> access{ast};
		auto changed = ast.Filter<CChanged>();
		if (access.Size<CChanged>() <= 0)
		{
			return;
		}

		TArray<AST::Id> callExprIds = AST::ListAll<CExprCallId>(access);
		AST::RemoveIf<CCallDirty>(access, callExprIds);
		for (AST::Id id : callExprIds)
		{
			const AST::Id functionId = access.Get<const CExprCallId>(id).functionId;
			if (!IsNone(functionId) && access.Has(functionId))
			{
				access.Add<CCallDirty>(id);
			}
		}
	}

	void SyncCallArguments(AST::Tree& ast)
	{
		// auto functionParams = ast.Filter<CExprType, CExprInput, CExprOutputs>();
		// auto identifiers    = ast.Filter<CIdentifier>();
		// auto exprInputs     = ast.Filter<CExprInput>();
		// auto exprOutputs    = ast.Filter<CExprOutputs>();

		TArray<CallToSync> calls;
		TAccess<CCallDirty, CExprCallId, TWrite<CExprInputs>, TWrite<CExprOutputs>,
		    TWrite<CInvalid>, TWrite<CExprType>, TWrite<CIdentifier>>
		    access{ast};
		for (AST::Id id : AST::ListAll<CCallDirty, CExprCallId>(access))
		{
			const auto& call = access.Get<const CExprCallId>(id);
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
		for (auto& call : calls)
		{
			auto& callOutputs = access.GetOrAdd<CExprOutputs>(call.id);
			// For each function pin
			for (i32 i = 0; i < call.functionOutputs->pinIds.Size(); ++i)
			{
				const AST::Id pinId = call.functionOutputs->pinIds[i];
				const auto* name    = access.TryGet<const CIdentifier>(pinId);
				if (!name)
				{
					continue;
				}

				const auto* pinType     = access.TryGet<const CExprType>(pinId);
				const AST::Id pinTypeId = pinType ? pinType->id : AST::NoId;

				if (i >= callOutputs.pinIds.Size())
				{
					AST::Id id = ast.Create();
					access.Add<CIdentifier>(id, *name);
					access.Add<CExprType>(id, {pinTypeId});
					callOutputs.Add(id);
					continue;
				}

				// Search matching pin to 'pinId' from i to end
				i32 callPinIdx = i;
				while (callPinIdx < callOutputs.pinIds.Size())
				{
					const AST::Id pinId     = callOutputs.pinIds[callPinIdx];
					const auto* callPinName = access.TryGet<const CIdentifier>(pinId);
					if (callPinName && *callPinName == *name)
						break;    // Found existing pin
					++callPinIdx;
				}
				if (callPinIdx == callOutputs.pinIds.Size())    // Pin not found, insert it
				{
					AST::Id id = ast.Create();
					access.Add<CIdentifier>(id, *name);
					access.Add<CExprType>(id, {pinTypeId});
					AST::Hierarchy::AddChildren(ast, call.id, id);
					callOutputs.Insert(i, id);
				}
				else if (callPinIdx > i)
				{
					// Correct pin is after where it should, we swap it to ensure correct order
					callOutputs.Swap(callPinIdx, i);
				}
			}

			// Mark as invalid all after N function params, and valid those before
			const i32 firstInvalid = call.functionOutputs->pinIds.Size();
			const i32 count = callOutputs.pinIds.Size() - call.functionOutputs->pinIds.Size();
			if (firstInvalid > 0)
			{
				access.Remove<CInvalid>({callOutputs.pinIds.Data(), firstInvalid});
			}
			if (count > 0)
			{
				access.Add<CInvalid>({callOutputs.pinIds.Data() + firstInvalid, count});
			}
		}

		/*for (auto& call : calls)
		{
		    for (AST::Id id : call.functionInputs->pinIds)
		    {
		        invalidOutputs.Empty(false);
		        const auto& name = access.Get<CIdentifier>(id);

		        const i32 idx = invalidOutputs.FindIndex([&access, &name](AST::Id id) {
		            return access.Get<CIdentifier>(id) == name;
		        });
		        if (idx != NO_INDEX)
		        {
		            call.inputArgs.Add(invalidOutputs[idx]);
		            currentInputs.RemoveAtSwap(idx, false);
		        }
		        else
		        {
		            AST::Id id = ast.Create();
		            access.Add<CIdentifier>(id, name);
		            // access.Add<CExprInput>(id);
		            call.inputArgs.Add(id);
		        }
		    }

		    call.invalidArgs.Append(currentInputs);
		    call.invalidArgs.Append(currentOutputs);
		}*/

		// for (auto& call : calls)
		//{
		//  AST::Hierarchy::RemoveAllChildren(ast, call.id, true);
		//  AST::Hierarchy::AddChildren(ast, call.id, call.inputArgs);
		//  AST::Hierarchy::AddChildren(ast, call.id, call.outputArgs);
		//  AST::Hierarchy::AddChildren(ast, call.id, call.invalidArgs);
		//  AST::Hierarchy::AddChildren(ast, call.id, call.unrelatedCallChildren);
		//}

		// RemoveInvalidDisconnectedArgs(ast);
	}

	/*void RemoveInvalidDisconnectedArgs(AST::Tree& ast)
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
	}*/

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CCallDirty>().Reset();
	}
}    // namespace Rift::FunctionsSystem
