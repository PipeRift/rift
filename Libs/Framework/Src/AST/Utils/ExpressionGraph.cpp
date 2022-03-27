// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/ExpressionGraph.h"

#include "AST/Components/CExprInput.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Types.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::ExpressionGraph
{
	bool CanConnect(const Tree& ast, AST::Id outputNode, AST::Id inputPin)
	{
		if (outputNode == inputPin)
		{
			return false;
		}

		if (!ast.Has<CExprOutputs>(outputNode) || !ast.Has<CExprInput>(inputPin))
		{
			return false;
		}

		// TODO: Ensure outputNode doesnt loop to inputPin
		return true;
	}

	bool TryConnect(Tree& ast, Id outputPin, Id inputPin)
	{
		if (!Ensure(!IsNone(outputPin) && !IsNone(inputPin)))
		{
			return false;
		}

		if (!CanConnect(ast, outputPin, inputPin))
		{
			return false;
		}

		// Link input
		{
			auto& inputComp = ast.Get<CExprInput>(inputPin);
			// Disconnect previous output connected to input if any
			if (inputComp.linkOutputPin != AST::NoId)
			{
				auto& lastOutputsComp = ast.Get<CExprOutputs>(inputComp.linkOutputPin);
				lastOutputsComp.linkInputPins.Remove(inputPin);
			}
			inputComp.linkOutputPin = outputPin;
		}

		// Link output
		{
			auto& outputsComp = ast.GetOrAdd<CExprOutputs>(outputPin);

			i32 index = outputsComp.linkInputPins.FindIndex(outputPin);
			if (index != NO_INDEX)
			{
				AST::Id& lastInputPin = outputsComp.linkInputPins[index];
				// Disconnect previous input connected to output if any
				if (lastInputPin != AST::NoId)
				{
					ast.Get<CExprInput>(lastInputPin).linkOutputPin = AST::NoId;
				}
				lastInputPin = inputPin;
			}
			else
			{
				// Pin didnt exist on the graph
				outputsComp.linkInputPins.Add(inputPin);
			}
		}
		return true;
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
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin)
	{
		// NOTE: Can be optimized if needed since outputs is accessed twice counting
		// Disconnect()
		if (auto* outputsComp = ast.TryGet<CExprOutputs>(outputPin))
		{
			i32 pinIndex = outputsComp->linkInputPins.FindIndex(outputPin);
			if (pinIndex != NO_INDEX) [[likely]]
			{
				return Disconnect(ast, outputsComp->linkInputPins[pinIndex]);
			}
		}
		return false;
	}
}    // namespace Rift::AST::ExpressionGraph
