// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/Expressions.h"

#include "AST/Id.h"


namespace rift::AST
{
	bool WouldExprLoop(
	    TAccessRef<CExprInputs, CExprOutputs, CExprTypeId> access, Id outputNodeId, Id inputNodeId)
	{
		TArray<Id> currentNodeIds{outputNodeId};
		TArray<Id> nextNodeIds{};
		while (!currentNodeIds.IsEmpty())
		{
			for (Id id : currentNodeIds)
			{
				if (const auto* inputs = access.TryGet<const CExprInputs>(id))
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

	bool CanConnectExpr(TAccessRef<CExprInputs, CExprOutputs, CExprTypeId> access,
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

		if (!access.Has<CExprOutputs>(output.nodeId) || !access.Has<CExprInputs>(input.nodeId))
		{
			return false;
		}

		if (!access.Get<const CExprOutputs>(output.nodeId).pinIds.Contains(output.pinId)
		    || !access.Get<const CExprInputs>(input.nodeId).pinIds.Contains(input.pinId))
		{
			return false;
		}

		{    // Type checking
			const auto* outputType = access.TryGet<const CExprTypeId>(output.pinId);
			// Can connect if output is any or not void
			if (outputType && outputType->id == NoId)
			{
				return false;
			}

			const auto* inputType = access.TryGet<const CExprTypeId>(input.pinId);
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
		return !WouldExprLoop(access, output.nodeId, input.nodeId);
	}

	bool TryConnectExpr(TAccessRef<TWrite<CExprInputs>, CExprOutputs, CExprTypeId> access,
	    ExprOutput output, ExprInput input)
	{
		if (!CanConnectExpr(access, output, input))
		{
			return false;
		}

		auto& inputs = access.Get<CExprInputs>(input.nodeId);

		// Find pin index
		const i32 index = inputs.pinIds.FindIndex([&input](Id pinId) {
			return input.pinId == pinId;
		});
		if (index != NO_INDEX && Ensure(index < inputs.linkedOutputs.Size()))
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
		const i32 index = inputs.pinIds.FindIndex([&input](Id pinId) {
			return input.pinId == pinId;
		});
		if (index != NO_INDEX && Ensure(index < inputs.linkedOutputs.Size())) [[likely]]
		{
			ExprOutput& linked = inputs.linkedOutputs[index];
			linked             = {};
			return true;
		}
		return false;
	}


	bool RemoveExprInputPin(TAccessRef<CExprInputs, TWrite<CInvalid>> access, ExprInput input)
	{
		if (!input.IsNone())
		{
			const auto* inputs = access.TryGet<const CExprInputs>(input.nodeId);
			if (inputs && inputs->pinIds.FindIndex(input.pinId) != NO_INDEX)
			{
				access.Add<CInvalid>(input.pinId);
				return true;
			}
		}
		return false;
	}

	bool RemoveExprOutputPin(TAccessRef<CExprOutputs, TWrite<CInvalid>> access, ExprOutput output)
	{
		if (!output.IsNone())
		{
			if (access.Has<CExprOutputs>(output.nodeId))
			{
				access.Add<CInvalid>(output.pinId);
				return true;
			}
		}
		return false;
	}

	ExprInput GetExprInputFromPin(TAccessRef<CExprInputs, CChild> access, Id pinId)
	{
		ExprInput input{};
		input.pinId = pinId;
		// If node is not the pin itself, it must be the parent
		input.nodeId = pinId;
		if (!IsNone(input.nodeId) && !access.Has<CExprInputs>(input.nodeId))
		{
			input.nodeId = p::GetIdParent(access, pinId);
		}
		return input;
	}

	ExprOutput GetExprOutputFromPin(TAccessRef<CExprOutputs, CChild> access, Id pinId)
	{
		ExprOutput output{};
		output.pinId = pinId;
		// If node is not the pin itself, it must be the parent
		output.nodeId = pinId;
		if (!IsNone(output.nodeId) && !access.Has<CExprOutputs>(output.nodeId))
		{
			output.nodeId = p::GetIdParent(access, pinId);
		}
		return output;
	}
}    // namespace rift::AST
