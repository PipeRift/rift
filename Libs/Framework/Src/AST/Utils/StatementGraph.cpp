// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/StatementGraph.h"

#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Types.h"


namespace Rift::AST::StatementGraph
{
	void Connect(Tree& ast, Id outputNode, Id outputPin, Id inputNode)
	{
		Check(!IsNone(outputNode) && !IsNone(outputPin) && !IsNone(inputNode));

		// Link input
		{
			auto& inputComp = ast.GetOrAdd<CStatementInput>(inputNode);
			// Disconnect previous output connected to input if any
			if (inputComp.edgeOutputNode != AST::NoId)
			{
				auto& lastOutputsComp = ast.Get<CStatementOutputs>(inputComp.edgeOutputNode);
				lastOutputsComp.edgePins.FindRef(outputPin) = AST::NoId;
			}
			inputComp.edgeOutputNode = outputNode;
		}

		// Link output
		{
			auto& outputsComp  = ast.GetOrAdd<CStatementOutputs>(outputNode);
			const i32 pinIndex = outputsComp.edgePins.FindIndex(outputPin);
			if (pinIndex != NO_INDEX)
			{
				Id& lastInputNode = outputsComp.edgeInputNodes[pinIndex];
				// Disconnect previous input connected to output if any
				if (lastInputNode != AST::NoId)
				{
					ast.Get<CStatementInput>(lastInputNode).edgeOutputNode = AST::NoId;
				}
				lastInputNode = inputNode;
				return;
			}
			// Pin didnt exist on the graph
			outputsComp.edgePins.Add(outputPin);
			outputsComp.edgeInputNodes.Add(inputNode);
		}
	}

	bool Disconnect(Tree& ast, Id edgeId)
	{
		Check(!IsNone(edgeId));

		const Id inputNode = edgeId;
		if (auto* inputComp = ast.TryGet<CStatementInput>(inputNode))
		{
			if (EnsureMsg(IsNone(inputComp->edgeOutputNode),
			        "Trying to disconnect a disconnected node")) [[likely]]
			{
				// We expect the other side to have outputs component
				const auto& outputsComp = ast.Get<CStatementOutputs>(inputComp->edgeOutputNode);
				if (Id* lastInputNode = outputsComp.edgeInputNodes.Find(inputNode)) [[likely]]
				{
					*lastInputNode = AST::NoId;
				}
				inputComp->edgeOutputNode = AST::NoId;
				return true;
			}
		}
		return false;
	}
}    // namespace Rift::AST::StatementGraph
