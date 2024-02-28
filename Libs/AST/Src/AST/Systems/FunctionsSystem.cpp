// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Systems/FunctionsSystem.h"

#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Utils/Namespaces.h"
#include "AST/Utils/TypeUtils.h"

#include <PipeECS.h>


namespace rift::ast::FunctionsSystem
{
	struct CallToSync
	{
		Id id;

		Id functionId;
		CExprInputs* functionInputs;
		CExprOutputs* functionOutputs;

		p::TArray<Id> inputArgs;
		p::TArray<Id> outputArgs;
		p::TArray<Id> invalidArgs;
		p::TArray<Id> unrelatedCallChildren;
	};


	void Init(Tree& ast)
	{
		ast.OnAdd<CExprCallId>().Bind([](auto& ast, auto ids) {
			ast.template AddN<CCallDirty>(ids);
		});
	}

	void ResolveCallFunctionIds(
	    p::TAccessRef<p::TWrite<CExprCallId>, CExprCall, CDeclFunction, CNamespace, CParent, CChild>
	        access)
	{
		auto callExprs = FindAllIdsWith<CExprCall>(access);
		ExcludeIdsWith<CExprCallId>(access, callExprs);
		for (Id id : callExprs)
		{
			auto& call          = access.Get<const CExprCall>(id);
			const Id functionId = FindIdFromNamespace(access, call.function);
			if (!IsNone(functionId))
			{
				access.Add(id, CExprCallId{functionId});
			}
		}
	}

	void PropagateDirtyIntoCalls(Tree& ast)
	{
		p::TAccess<CChanged, CExprCallId, p::TWrite<CCallDirty>> access{ast};
		if (access.Size<CChanged>() <= 0)
		{
			return;
		}

		p::TArray<Id> callExprIds = p::FindAllIdsWith<CExprCallId>(access);
		p::ExcludeIdsWith<CCallDirty>(access, callExprIds);
		for (Id id : callExprIds)
		{
			const Id functionId = access.Get<const CExprCallId>(id).functionId;
			if (!IsNone(functionId) && access.Has<CChanged>(functionId))
			{
				access.Add<CCallDirty>(id);
			}
		}
	}

	void PushInvalidPinsBack(
	    p::TAccessRef<p::TWrite<CExprInputs>, p::TWrite<CExprOutputs>, CInvalid> access)
	{
		for (Id inputsId : p::FindAllIdsWith<CExprInputs>(access))
		{
			auto& inputs     = access.Get<CExprInputs>(inputsId);
			p::i32 validSize = inputs.pinIds.Size();
			for (p::i32 i = 0; i < validSize;)
			{
				Id id = inputs.pinIds[i];
				if (access.Has<CInvalid>(id))
				{
					ExprOutput output = inputs.linkedOutputs[i];
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

		for (Id outputsId : p::FindAllIdsWith<CExprOutputs>(access))
		{
			auto& outputs    = access.Get<CExprOutputs>(outputsId);
			p::i32 validSize = outputs.pinIds.Size();
			for (p::i32 i = 0; i < validSize;)
			{
				Id id = outputs.pinIds[i];
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

	void SyncCallPinsFromFunction(Tree& ast)
	{
		p::TArray<CallToSync> calls;
		p::TAccess<CCallDirty, CExprCallId, p::TWrite<CExprInputs>, p::TWrite<CExprOutputs>,
		    p::TWrite<CInvalid>, p::TWrite<CExprTypeId>, p::TWrite<CNamespace>>
		    access{ast};
		for (Id id : p::FindAllIdsWith<CCallDirty, CExprCallId>(access))
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
			p::i32 validSize = call.functionInputs->pinIds.Size();
			for (p::i32 i = 0; i < validSize; ++i)
			{
				const Id pinId = call.functionInputs->pinIds[i];
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
					Id id = ast.Create();
					access.Add<CNamespace>(id, *name);
					p::AttachId(ast, call.id, id);
					callOutputs.Add(id);
				}
				else
				{
					// Search matching pin to 'pinId' from i to end
					p::i32 callPinIdx = i;
					while (callPinIdx < callOutputs.pinIds.Size())
					{
						const Id outputPinId    = callOutputs.pinIds[callPinIdx];
						const auto* callPinName = access.TryGet<const CNamespace>(outputPinId);
						if (callPinName && *callPinName == *name)
							break;    // Found existing pin
						++callPinIdx;
					}
					if (callPinIdx == callOutputs.pinIds.Size())    // Pin not found, insert it
					{
						Id id = ast.Create();
						access.Add<CNamespace>(id, *name);
						p::AttachId(ast, call.id, id);
						callOutputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callOutputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType = access.TryGet<const CExprTypeId>(pinId);
				access.Add<CExprTypeId>(callOutputs.pinIds[i], pinType ? *pinType : CExprTypeId{});
			}

			// Mark as invalid all after N function params, and valid those before
			const p::i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				access.Remove<CInvalid>({callOutputs.pinIds.Data(), firstInvalid});
			}
			const p::i32 count = callOutputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				access.AddN<CInvalid>({callOutputs.pinIds.Data() + firstInvalid, count});
			}
		}

		// Sync call inputs to function outputs
		for (auto& call : calls)
		{
			auto& callInputs = access.GetOrAdd<CExprInputs>(call.id);
			// For each function pin
			p::i32 validSize = call.functionOutputs->pinIds.Size();
			for (p::i32 i = 0; i < validSize; ++i)
			{
				const Id pinId = call.functionOutputs->pinIds[i];
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
					Id id = ast.Create();
					access.Add<CNamespace>(id, *name);
					p::AttachId(ast, call.id, id);
					callInputs.Add(id);
				}
				else
				{
					// Search matching pin to 'pinId' from i to end
					p::i32 callPinIdx = i;
					while (callPinIdx < callInputs.pinIds.Size())
					{
						const Id pinId          = callInputs.pinIds[callPinIdx];
						const auto* callPinName = access.TryGet<const CNamespace>(pinId);
						if (callPinName && *callPinName == *name)
							break;    // Found existing pin
						++callPinIdx;
					}
					if (callPinIdx == callInputs.pinIds.Size())    // Pin not found, insert it
					{
						Id id = ast.Create();
						access.Add<CNamespace>(id, *name);
						p::AttachId(ast, call.id, id);
						callInputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callInputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType = access.TryGet<const CExprTypeId>(pinId);
				access.Add<CExprTypeId>(callInputs.pinIds[i], pinType ? *pinType : CExprTypeId{});
			}

			// Mark as invalid all after N function params, and valid those before
			const p::i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				access.Remove<CInvalid>({callInputs.pinIds.Data(), firstInvalid});
			}
			const p::i32 count = callInputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				access.AddN<CInvalid>({callInputs.pinIds.Data() + firstInvalid, count});
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

		for (Id id : FindAllIdsWith<CExprInputs>(access))
		{
			const auto& inputs = access.Get<const CExprInputs>(id);
			for (p::i32 i = 0; i < inputs.pinIds.Size(); ++i)
			{
				Id pinId                 = inputs.pinIds[i];
				const ExprOutput& output = inputs.linkedOutputs[i];
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

		p::TArray<Id> pinsToRemove = p::FindAllIdsWith<CInvalid>(access);
		ExcludeIdsWith<CTmpInvalidKeep>(access, pinsToRemove);
		p::RemoveId(access, pinsToRemove);

		access.GetPool<CTmpInvalidKeep>()->Clear();
	}

	void ClearAddedTags(Tree& ast)
	{
		ast.AssurePool<CCallDirty>().Clear();
	}
}    // namespace rift::ast::FunctionsSystem
