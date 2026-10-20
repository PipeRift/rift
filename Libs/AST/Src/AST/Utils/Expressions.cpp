// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Utils/Expressions.h"

#include "AST/Id.h"
#include "AST/Systems/CastSystem.h"


namespace rift::ast
{
	bool WouldExprLoop(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    Id outputNodeId, Id inputNodeId)
	{
		// Walk upstream from the output node. If the input node feeds it, the link would loop.
		// The array doubles as the queue and the visited set, so each node is walked once.
		p::TArray<Id> nodeIds{outputNodeId};
		for (p::i32 i = 0; i < nodeIds.Size(); ++i)
		{
			const auto* inputs = scope.TryGet<const CExprInputs>(nodeIds[i]);
			if (!inputs)
			{
				continue;
			}

			for (const ExprOutput& output : inputs->linkedOutputs)
			{
				if (output.nodeId == inputNodeId)
				{
					return true;
				}
				else if (!IsNone(output.nodeId))
				{
					nodeIds.Add(output.nodeId);
				}
			}
		}
		return false;
	}

	bool CanConnectExpr(p::TIdScopeRef<CExprInputs, CExprOutputs, CExprTypeId> scope,
	    const ExprOutput& output, const ExprInput& input)
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
				// Different types can still be connected when the value can be cast into the
				// type of the input on its own
				if (outputType->id != inputType->id
				    && !CastSystem::CanCastImplicitly(
				        static_cast<Tree&>(scope.GetContext()), outputType->id, inputType->id))
				{
					return false;
				}
			}
		}

		// Ensure output and input wouldn't loop
		return !WouldExprLoop(scope, output.nodeId, input.nodeId);
	}

	bool TryConnectExpr(p::TIdScopeRef<p::Writes<CExprInputs>, CExprOutputs, CExprTypeId> scope,
	    const ExprOutput& output, ExprInput input)
	{
		if (!CanConnectExpr(scope, output, input))
		{
			return false;
		}

		auto& inputs = scope.Get<CExprInputs>(input.nodeId);

		// Find pin index
		const p::i32 index = inputs.pinIds.FindIndex(input.pinId);
		if (index != p::NO_INDEX && P_Ensure(index < inputs.linkedOutputs.Size()))
		{
			inputs.linkedOutputs[index] = output;
			return true;
		}
		return false;    // Pin was invalid
	}

	bool DisconnectExpr(Tree& ast, const ExprInput& input)
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


	bool RemoveExprInputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprInputs> scope, const ExprInput& input)
	{
		if (!input.IsNone())
		{
			const auto* inputs = scope.TryGet<const CExprInputs>(input.nodeId);
			if (inputs && inputs->pinIds.Contains(input.pinId))
			{
				scope.Add<CInvalid>(input.pinId);
				return true;
			}
		}
		return false;
	}

	bool RemoveExprOutputPin(
	    p::TIdScopeRef<p::Writes<CInvalid>, CExprOutputs> scope, const ExprOutput& output)
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

	Id GetPinType(p::TIdScopeRef<CExprTypeId> scope, Id pinId)
	{
		const auto* type = scope.TryGet<const CExprTypeId>(pinId);
		return type ? type->id : NoId;
	}
}    // namespace rift::ast
