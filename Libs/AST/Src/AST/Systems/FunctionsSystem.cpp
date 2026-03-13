// Copyright 2015-2026 Piperift. All Rights Reserved.

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


	void Init(Tree& ast) {}

	void ResolveCallFunctionIds(p::TIdScopeRef<p::Writes<CExprCallId>, CExprCall, CDeclFunction,
	    CNamespace, CParent, CChild>
	        scope)
	{
		auto callExprs = FindAllIdsWith<CExprCall>(scope);
		ExcludeIdsWith<CExprCallId>(scope, callExprs);
		for (Id id : callExprs)
		{
			auto& call          = scope.Get<const CExprCall>(id);
			const Id functionId = FindIdFromNamespace(scope, call.function);
			if (!IsNone(functionId))
			{
				scope.Add(id, CExprCallId{functionId});
			}
		}
	}

	void PropagateDirtyIntoCalls(Tree& ast)
	{
		p::TIdScope<p::Writes<p::CMdfd<CExprCallId>>, CChanged, CExprCallId> scope{ast};
		if (scope.Size<CChanged>() <= 0)
		{
			return;
		}

		p::TArray<Id> callExprIds = p::FindAllIdsWith<CExprCallId>(scope);
		p::ExcludeIdsWith<p::CMdfd<CExprCallId>>(scope, callExprIds);
		for (Id id : callExprIds)
		{
			const Id functionId = scope.Get<const CExprCallId>(id).functionId;
			if (!IsNone(functionId) && scope.Has<CChanged>(functionId))
			{
				scope.Add<p::CMdfd<CExprCallId>>(id);
			}
		}
	}

	void PushInvalidPinsBack(p::TIdScopeRef<p::Writes<CExprInputs, CExprOutputs>, CInvalid> scope)
	{
		for (Id inputsId : p::FindAllIdsWith<CExprInputs>(scope))
		{
			auto& inputs     = scope.Get<CExprInputs>(inputsId);
			p::i32 validSize = inputs.pinIds.Size();
			for (p::i32 i = 0; i < validSize;)
			{
				Id id = inputs.pinIds[i];
				if (scope.Has<CInvalid>(id))
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

		for (Id outputsId : p::FindAllIdsWith<CExprOutputs>(scope))
		{
			auto& outputs    = scope.Get<CExprOutputs>(outputsId);
			p::i32 validSize = outputs.pinIds.Size();
			for (p::i32 i = 0; i < validSize;)
			{
				Id id = outputs.pinIds[i];
				if (scope.Has<CInvalid>(id))
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
		p::TIdScope<p::Writes<CExprInputs, CExprOutputs, CInvalid, CExprTypeId, CNamespace>,
		    p::CMdfd<CExprCallId>, CExprCallId>
		    scope{ast};
		for (Id id : p::FindAllIdsWith<p::CMdfd<CExprCallId>, CExprCallId>(scope))
		{
			const auto& call = scope.Get<const CExprCallId>(id);
			if (IsNone(call.functionId))
			{
				continue;
			}

			CallToSync cache;
			cache.id              = id;
			cache.functionId      = call.functionId;
			cache.functionOutputs = &scope.GetOrAdd<CExprOutputs>(call.functionId);
			cache.functionInputs  = &scope.GetOrAdd<CExprInputs>(call.functionId);
			calls.Add(cache);
		}

		// Sync call outputs to function inputs
		for (auto& call : calls)
		{
			auto& callOutputs = scope.GetOrAdd<CExprOutputs>(call.id);


			// For each function pin
			p::i32 validSize = call.functionInputs->pinIds.Size();
			for (p::i32 i = 0; i < validSize; ++i)
			{
				const Id pinId = call.functionInputs->pinIds[i];
				if (scope.Has<CInvalid>(pinId))
				{
					validSize = i;
					break;
				}

				const auto* name = scope.TryGet<const CNamespace>(pinId);
				if (!name)
				{
					continue;
				}

				if (i >= callOutputs.pinIds.Size())
				{
					Id id = p::AddId(ast);
					scope.Add<CNamespace>(id, *name);
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
						const auto* callPinName = scope.TryGet<const CNamespace>(outputPinId);
						if (callPinName && *callPinName == *name)
						{
							break;    // Found existing pin
						}
						++callPinIdx;
					}
					if (callPinIdx == callOutputs.pinIds.Size())    // Pin not found, insert it
					{
						Id id = p::AddId(ast);
						scope.Add<CNamespace>(id, *name);
						p::AttachId(ast, call.id, id);
						callOutputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callOutputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType = scope.TryGet<const CExprTypeId>(pinId);
				scope.Add<CExprTypeId>(callOutputs.pinIds[i], pinType ? *pinType : CExprTypeId{});
			}

			// Mark as invalid all after N function params, and valid those before
			const p::i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				scope.Remove<CInvalid>({callOutputs.pinIds.Data(), firstInvalid});
			}
			const p::i32 count = callOutputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				scope.AddN<CInvalid>({callOutputs.pinIds.Data() + firstInvalid, count});
			}
		}

		// Sync call inputs to function outputs
		for (auto& call : calls)
		{
			auto& callInputs = scope.GetOrAdd<CExprInputs>(call.id);
			// For each function pin
			p::i32 validSize = call.functionOutputs->pinIds.Size();
			for (p::i32 i = 0; i < validSize; ++i)
			{
				const Id pinId = call.functionOutputs->pinIds[i];
				if (scope.Has<CInvalid>(pinId))
				{
					validSize = i;
					break;
				}

				const auto* name = scope.TryGet<const CNamespace>(pinId);
				if (!name)
				{
					continue;
				}

				if (i >= callInputs.pinIds.Size())
				{
					Id id = p::AddId(ast);
					scope.Add<CNamespace>(id, *name);
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
						const auto* callPinName = scope.TryGet<const CNamespace>(pinId);
						if (callPinName && *callPinName == *name)
						{
							break;    // Found existing pin
						}
						++callPinIdx;
					}
					if (callPinIdx == callInputs.pinIds.Size())    // Pin not found, insert it
					{
						Id id = p::AddId(ast);
						scope.Add<CNamespace>(id, *name);
						p::AttachId(ast, call.id, id);
						callInputs.Insert(i, id);
					}
					else if (callPinIdx > i)
					{
						// Correct pin is after where it should, we swap it to ensure correct order
						callInputs.Swap(callPinIdx, i);
					}
				}

				const auto* pinType = scope.TryGet<const CExprTypeId>(pinId);
				scope.Add<CExprTypeId>(callInputs.pinIds[i], pinType ? *pinType : CExprTypeId{});
			}

			// Mark as invalid all after N function params, and valid those before
			const p::i32 firstInvalid = validSize;
			if (firstInvalid > 0)
			{
				scope.Remove<CInvalid>({callInputs.pinIds.Data(), firstInvalid});
			}
			const p::i32 count = callInputs.pinIds.Size() - validSize;
			if (count > 0)
			{
				scope.AddN<CInvalid>({callInputs.pinIds.Data() + firstInvalid, count});
			}
		}

		RemoveInvalidDisconnectedArgs(ast);
	}

	void RemoveInvalidDisconnectedArgs(InvalidDisconnectedPinAccess scope)
	{
		if (scope.Size<CInvalid>() <= 0)
		{
			// No invalids!
			return;
		}

		for (Id id : FindAllIdsWith<CExprInputs>(scope))
		{
			const auto& inputs = scope.Get<const CExprInputs>(id);
			for (p::i32 i = 0; i < inputs.pinIds.Size(); ++i)
			{
				Id pinId                 = inputs.pinIds[i];
				const ExprOutput& output = inputs.linkedOutputs[i];
				if (!output.IsNone())    // Is connected
				{
					if (scope.Has<CInvalid>(pinId))
					{
						scope.Add<CTmpInvalidKeep>(pinId);
					}
					if (scope.Has<CInvalid>(output.pinId))
					{
						scope.Add<CTmpInvalidKeep>(output.pinId);
					}
				}
				else
				{
					// if (scope.Has<CInvalid>(pinId))
					//{
					//	// Remove invalid disconnected input
					//	inputs.pinIds.RemoveAt(i);
					//	inputs.linkedOutputs.RemoveAt(i);
					// }
				}
			}
		}

		p::TArray<Id> pinsToRemove = p::FindAllIdsWith<CInvalid>(scope);
		ExcludeIdsWith<CTmpInvalidKeep>(scope, pinsToRemove);
		p::RmId(scope.GetContext(), pinsToRemove);

		scope.GetPool<CTmpInvalidKeep>()->Clear();
	}

	void ClearAddedTags(Tree& ast)
	{
		ast.AssurePool<p::CMdfd<CExprCallId>>().Clear();
	}
}    // namespace rift::ast::FunctionsSystem
