// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Expressions.h"

#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Types.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Expressions
{
	bool WouldLoop(
	    TAccessRef<CExprInputs, CExprOutputs> access, AST::Id outputNodeId, AST::Id inputNodeId)
	{
		TArray<AST::Id> currentNodeIds{outputNodeId};
		TArray<AST::Id> nextNodeIds{};
		while (!currentNodeIds.IsEmpty())
		{
			for (AST::Id id : currentNodeIds)
			{
				if (const auto* inputs = access.TryGet<const CExprInputs>(id))
				{
					nextNodeIds.ReserveMore(inputs->linkedOutputs.Size());
					for (OutputId output : inputs->linkedOutputs)
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
			nextNodeIds.Empty(false);
		}
		return false;
	}

	bool CanConnect(TAccessRef<CExprInputs, CExprOutputs> access, OutputId output, InputId input)
	{
		if (IsNone(output.nodeId) || IsNone(output.pinId) || IsNone(input.nodeId)
		    || IsNone(input.pinId))
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

		// Ensure output and input wouldn't loop
		return !WouldLoop(access, output.nodeId, input.nodeId);
	}

	bool TryConnect(
	    TAccessRef<TWrite<CExprInputs>, CExprOutputs> access, OutputId output, InputId input)
	{
		if (!CanConnect(access, output, input))
		{
			return false;
		}

		// Link input
		auto& inputs = access.Get<CExprInputs>(input.nodeId);

		const i32 index = inputs.pinIds.FindIndex([&input](AST::Id pinId) {
			return input.pinId == pinId;
		});

		if (index != NO_INDEX && Ensure(index < inputs.linkedOutputs.Size()))
		{
			inputs.linkedOutputs[index] = output;
			return true;
		}
		return false;    // Pin was invalid
	}

	bool Disconnect(Tree& ast, Id linkId)
	{
		if (IsNone(linkId))
		{
			return false;
		}

		const Id inputPin = linkId;
		auto* inputComp   = ast.TryGet<CExprInput>(inputPin);
		if (EnsureMsg(inputComp && !IsNone(inputComp->linkOutputPin),
		        "Trying to disconnect a unexistant link")) [[likely]]
		{
			// We expect the other side to have outputs component
			auto& outputsComp = ast.Get<CExprOutputs>(inputComp->linkOutputPin);
			if (Id* lastInputPin = outputsComp.linkInputPins.Find(inputPin)) [[likely]]
			{
				*lastInputPin = AST::NoId;
			}
			inputComp->linkOutputPin = AST::NoId;
			return true;
		}
		return false;
	}

	bool DisconnectFromInputPin(Tree& ast, AST::Id inputPin)
	{
		// NOTE: Input pin ids equal input node ids
		return Disconnect(ast, inputPin);
	}
}    // namespace Rift::AST::Expressions
