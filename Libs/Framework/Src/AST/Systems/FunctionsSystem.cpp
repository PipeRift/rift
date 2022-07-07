// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/TypeUtils.h"

#include <Pipe/ECS/Filtering.h>


namespace rift::FunctionsSystem
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

	void ResolveCallFunctionIds(
	    TAccessRef<TWrite<CExprCallId>, CExprCall, CDeclFunction, CNamespace, CParent> access)
	{
		auto callExprs = ecs::ListAny<CExprCall>(access);
		ecs::ExcludeIf<CExprCallId>(access, callExprs);
		for (AST::Id id : callExprs)
		{
			auto& call = access.Get<const CExprCall>(id);
			AST::Id functionId =
			    Types::FindFunctionByName(access, call.nameSpace, call.functionName);
			if (!IsNone(functionId))
			{
				access.Add(id, CExprCallId{functionId});
			}
		}
	}

	void PropagateDirtyIntoCalls(AST::Tree& ast)
	{
		TAccess<CChanged, CExprCallId, TWrite<CCallDirty>> access{ast};
		if (access.Size<CChanged>() <= 0)
		{
			return;
		}

		TArray<AST::Id> callExprIds = ecs::ListAll<CExprCallId>(access);
		ecs::ExcludeIf<CCallDirty>(access, callExprIds);
		for (AST::Id id : callExprIds)
		{
			const AST::Id functionId = access.Get<const CExprCallId>(id).functionId;
			if (!IsNone(functionId) && access.Has<CChanged>(functionId))
			{
				access.Add<CCallDirty>(id);
			}
		}
	}

	void PushInvalidPinsBack(TAccessRef<TWrite<CExprInputs>, TWrite<CExprOutputs>, CInvalid> access)
	{
		for (AST::Id inputsId : ecs::ListAll<CExprInputs>(access))
		{
			auto& inputs  = access.Get<CExprInputs>(inputsId);
			i32 validSize = inputs.pinIds.Size();
			for (i32 i = 0; i < validSize;)
			{
				AST::Id id = inputs.pinIds[i];
				if (access.Has<CInvalid>(id))
				{
					OutputId output = inputs.linkedOutputs[i];
					inputs.pinIds.RemoveAt(i, false);
					inputs.pinIds.Add(id);
					inputs.linkedOutputs.RemoveAt(i, false);
					inputs.linkedOutputs.Add(output);
					--validSize;
				}
				else
				{
					++i;
				}
			}
		}

		for (AST::Id outputsId : ecs::ListAll<CExprOutputs>(access))
		{
			auto& outputs = access.Get<CExprOutputs>(outputsId);
			i32 validSize = outputs.pinIds.Size();
			for (i32 i = 0; i < validSize;)
			{
				AST::Id id = outputs.pinIds[i];
				if (access.Has<CInvalid>(id))
				{
					outputs.pinIds.RemoveAt(i, false);
					outputs.pinIds.Add(id);
					--validSize;
				}
				else
				{
					++i;
				}
			}
		}
	}

	void SyncCallPinsFromFunction(AST::Tree& ast)
	{
		TArray<CallToSync> calls;
		TAccess<CCallDirty, CExprCallId, TWrite<CExprInputs>, TWrite<CExprOutputs>,
		    TWrite<CInvalid>, TWrite<CExprType>, TWrite<CNamespace>>
		    access{ast};
		for (AST::Id id : ecs::ListAll<CCallDirty, CExprCallId>(access))
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

		// Sync call outputs to function inputs
		for (auto& call : calls)
		{
			auto& callOutputs = access.GetOrAdd<CExprOutputs>(call.id);


			// For each function pin
			i32 validSize = call.functionInputs->pinIds.Size();
			for (i32 i = 0; i < validSize; ++i)
			{
				const AST::Id pinId = call.functionInputs->pinIds[i];
				if (access.Has<CInvalid>(pinId))
				{
					validSize = i;
					break;
				}

				const auto* name = access.TryGet<const CNamespace>(pinId);
				if (!name)
				{
					continue;
				}

				if (i >= callOutputs.pinIds.Size())
				{
					AST::Id id = ast.Create();
					access.Add<CNamespace>(id, *name);
					AST::Hierarchy::AddChildren(ast, call.id, id);
					callOutputs.Add(id);
				}
				else
				{
					// Search matching pin to 'pinId' from i to end
					i32 callPinIdx = i;
					while (callPinIdx < callOutputs.pinIds.Size())
					{
						const AST::Id outputPinId = callOutputs.pinIds[callPinIdx];
						const auto* callPinName   = access.TryGet<const CNamespace>(outputPinId);
						if (callPinName && *callPinName == *name)
							break;    // Found existing pin
						++callPinIdx;
					}
					if (callPinIdx == callOutputs.pinIds.Size())    // Pin not found, insert it
					{
						AST::Id id = ast.Create();
						access.Add<CNamespace>(id, *name);
						AST::Hierarchy::AddChildren(ast, call.id, id);
						callOutputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callOutputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType     = access.TryGet<const CExprType>(pinId);
				const AST::Id pinTypeId = pinType ? pinType->id : AST::NoId;
				access.Add<CExprType>(callOutputs.pinIds[i], {pinTypeId});
			}

			// Mark as invalid all after N function params, and valid those before
			const i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				access.Remove<CInvalid>({callOutputs.pinIds.Data(), firstInvalid});
			}
			const i32 count = callOutputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				access.Add<CInvalid>({callOutputs.pinIds.Data() + firstInvalid, count});
			}
		}

		// Sync call inputs to function outputs
		for (auto& call : calls)
		{
			auto& callInputs = access.GetOrAdd<CExprInputs>(call.id);
			// For each function pin
			i32 validSize = call.functionOutputs->pinIds.Size();
			for (i32 i = 0; i < validSize; ++i)
			{
				const AST::Id pinId = call.functionOutputs->pinIds[i];
				if (access.Has<CInvalid>(pinId))
				{
					validSize = i;
					break;
				}

				const auto* name = access.TryGet<const CNamespace>(pinId);
				if (!name)
				{
					continue;
				}

				if (i >= callInputs.pinIds.Size())
				{
					AST::Id id = ast.Create();
					access.Add<CNamespace>(id, *name);
					AST::Hierarchy::AddChildren(ast, call.id, id);
					callInputs.Add(id);
				}
				else
				{
					// Search matching pin to 'pinId' from i to end
					i32 callPinIdx = i;
					while (callPinIdx < callInputs.pinIds.Size())
					{
						const AST::Id pinId     = callInputs.pinIds[callPinIdx];
						const auto* callPinName = access.TryGet<const CNamespace>(pinId);
						if (callPinName && *callPinName == *name)
							break;    // Found existing pin
						++callPinIdx;
					}
					if (callPinIdx == callInputs.pinIds.Size())    // Pin not found, insert it
					{
						AST::Id id = ast.Create();
						access.Add<CNamespace>(id, *name);
						AST::Hierarchy::AddChildren(ast, call.id, id);
						callInputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callInputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType     = access.TryGet<const CExprType>(pinId);
				const AST::Id pinTypeId = pinType ? pinType->id : AST::NoId;
				access.Add<CExprType>(callInputs.pinIds[i], {pinTypeId});
			}

			// Mark as invalid all after N function params, and valid those before
			const i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				access.Remove<CInvalid>({callInputs.pinIds.Data(), firstInvalid});
			}
			const i32 count = callInputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				access.Add<CInvalid>({callInputs.pinIds.Data() + firstInvalid, count});
			}
		}

		RemoveInvalidDisconnectedArgs(ast);
	}

	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess access)
	{
		if (access.Size<CInvalid>() <= 0)
		{
			// No invalids!
			return;
		}

		for (AST::Id id : ecs::ListAll<CExprInputs>(access))
		{
			const auto& inputs = access.Get<const CExprInputs>(id);
			for (i32 i = 0; i < inputs.pinIds.Size(); ++i)
			{
				AST::Id pinId          = inputs.pinIds[i];
				const OutputId& output = inputs.linkedOutputs[i];
				if (!output.IsNone())    // Is connected
				{
					if (access.Has<CInvalid>(pinId))
					{
						access.Add<CTmpInvalidKeep>(pinId);
					}
					if (access.Has<CInvalid>(output.pinId))
					{
						access.Add<CTmpInvalidKeep>(output.pinId);
					}
				}
				else
				{
					// if (access.Has<CInvalid>(pinId))
					//{
					//	// Remove invalid disconnected input
					//	inputs.pinIds.RemoveAt(i);
					//	inputs.linkedOutputs.RemoveAt(i);
					// }
				}
			}
		}

		TArray<AST::Id> pinsToRemove = ecs::ListAll<CInvalid>(access);
		ecs::ExcludeIf<CTmpInvalidKeep>(access, pinsToRemove);
		AST::Hierarchy::Remove(access, pinsToRemove);

		access.GetPool<CTmpInvalidKeep>()->Reset();
	}

	void ClearAddedTags(AST::Tree& ast)
	{
		ast.AssurePool<CCallDirty>().Reset();
	}
}    // namespace rift::FunctionsSystem
