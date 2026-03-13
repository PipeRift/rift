// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Utils/Expressions.h"

#include "AST/Id.h"


namespace rift::ast
{
	bool WouldExprLoop(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    Id outputNodeId, Id inputNodeId)
	{
		p::TArray<Id> currentNodeIds{outputNodeId};
		p::TArray<Id> nextNodeIds{};
		while (!currentNodeIds.IsEmpty())
		{
			for (Id id : currentNodeIds)
			{
				if (const auto* inputs = scope.TryGet<const CExprInputs>(id))
				{
					nextNodeIds.ReserveMore(inputs->linkedOutputs.Size());
					for (ExprOutput output : inputs->linkedOutputs)
					{
						if (output.nodeId == inputNodeId)
						{
							return true;
						}
						else if (!IsNone(output.nodeId))
						{
							nextNodeIds.Add(output.nodeId);
						}
					}
				}
			}
			currentNodeIds = nextNodeIds;
			nextNodeIds.Clear(false);
		}
		return false;
	}

	bool CanConnectExpr(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    ExprOutput output, ExprInput input)
	{
		if (output.IsNone() || input.IsNone())
		{
			return false;
		}

		if (output.nodeId == input.nodeId || output.pinId == input.pinId)
		{
			return false;    // Can't connect to same node or same pin
		}

		if (!scope.Has<CExprOutputs>(output.nodeId) || !scope.Has<CExprInputs>(input.nodeId))
		{
			return false;
		}

		if (!scope.Get<const CExprOutputs>(output.nodeId).pinIds.Contains(output.pinId)
		    || !scope.Get<const CExprInputs>(input.nodeId).pinIds.Contains(input.pinId))
		{
			return false;
		}

		{    // Type checking
			const auto* outputType = scope.TryGet<const CExprTypeId>(output.pinId);
			// Can connect if output is any or not void
			if (outputType && outputType->id == NoId)
			{
				return false;
			}

			const auto* inputType = scope.TryGet<const CExprTypeId>(input.pinId);
			// Can connect if input is any or not void
			if (inputType && inputType->id == NoId)
			{
				return false;
			}

			if (outputType && inputType)
			{
				// TODO: Check if different types can be implicitly casted
				if (outputType->id != inputType->id)
				{
					return false;
				}
			}
		}

		// Ensure output and input wouldn't loop
		return !WouldExprLoop(scope, output.nodeId, input.nodeId);
	}

	bool TryConnectExpr(p::TIdScopeRef<p::Writes<CExprInputs>, CExprOutputs, CExprTypeId> scope,
	    ExprOutput output, ExprInput input)
	{
		if (!CanConnectExpr(scope, output, input))
		{
			return false;
		}

		auto& inputs = scope.Get<CExprInputs>(input.nodeId);

		// Find pin index
		const p::i32 index = inputs.pinIds.FindIndexIf([&input](Id pinId) {
			return input.pinId == pinId;
		});
		if (index != p::NO_INDEX && P_Ensure(index < inputs.linkedOutputs.Size()))
		{
			inputs.linkedOutputs[index] = output;
			return true;
		}
		return false;    // Pin was invalid
	}

	bool DisconnectExpr(Tree& ast, ExprInput input)
	{
		if (input.IsNone() || !ast.Has<CExprInputs>(input.nodeId))
		{
			return false;
		}

		auto& inputs = ast.Get<CExprInputs>(input.nodeId);

		// Find pin index
		const p::i32 index = inputs.pinIds.FindIndex(input.pinId);
		if (index != p::NO_INDEX && P_Ensure(index < inputs.linkedOutputs.Size())) [[likely]]
		{
			ExprOutput& linked = inputs.linkedOutputs[index];
			linked             = {};
			return true;
		}
		return false;
	}


	bool RemoveExprInputPin(p::TIdScopeRef<p::Writes<CInvalid>, CExprInputs> scope, ExprInput input)
	{
		if (!input.IsNone())
		{
			const auto* inputs = scope.TryGet<const CExprInputs>(input.nodeId);
			if (inputs && inputs->pinIds.FindIndex(input.pinId) != p::NO_INDEX)
			{
				scope.Add<CInvalid>(input.pinId);
				return true;
			}
		}
		return false;
	}

	bool RemoveExprOutputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprOutputs> scope, ExprOutput output)
	{
		if (!output.IsNone())
		{
			if (scope.Has<CExprOutputs>(output.nodeId))
			{
				scope.Add<CInvalid>(output.pinId);
				return true;
			}
		}
		return false;
	}

	ExprInput GetExprInputFromPin(p::TIdScopeRef<CExprInputs, CChild> scope, Id pinId)
	{
		ExprInput input{};
		input.pinId = pinId;
		// If node is not the pin itself, it must be the parent
		input.nodeId = pinId;
		if (!IsNone(input.nodeId) && !scope.Has<CExprInputs>(input.nodeId))
		{
			input.nodeId = p::GetIdParent(scope, pinId);
		}
		return input;
	}

	ExprOutput GetExprOutputFromPin(p::TIdScopeRef<CExprOutputs, CChild> scope, Id pinId)
	{
		ExprOutput output{};
		output.pinId = pinId;
		// If node is not the pin itself, it must be the parent
		output.nodeId = pinId;
		if (!IsNone(output.nodeId) && !scope.Has<CExprOutputs>(output.nodeId))
		{
			output.nodeId = p::GetIdParent(scope, pinId);
		}
		return output;
	}
}    // namespace rift::ast
