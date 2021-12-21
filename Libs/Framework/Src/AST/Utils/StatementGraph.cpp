// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/StatementGraph.h"

#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Types.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::StatementGraph
{
	void Connect(Tree& ast, Id outputNode, Id outputPin, Id inputNode)
	{
		if (!Ensure(!IsNone(outputNode) && !IsNone(outputPin) && !IsNone(inputNode)))
		{
			return;
		}

		// Link input
		{
			auto& inputComp = ast.GetOrAdd<CStatementInput>(inputNode);
			// Disconnect previous output connected to input if any
			if (inputComp.linkOutputNode != AST::NoId)
			{
				auto& lastOutputsComp = ast.Get<CStatementOutputs>(inputComp.linkOutputNode);
				lastOutputsComp.linkPins.FindRef(outputPin) = AST::NoId;
			}
			inputComp.linkOutputNode = outputNode;
		}

		// Link output
		{
			auto& outputsComp  = ast.GetOrAdd<CStatementOutputs>(outputNode);
			const i32 pinIndex = outputsComp.linkPins.FindIndex(outputPin);
			if (pinIndex != NO_INDEX)
			{
				Id& lastInputNode = outputsComp.linkInputNodes[pinIndex];
				// Disconnect previous input connected to output if any
				if (lastInputNode != AST::NoId)
				{
					ast.Get<CStatementInput>(lastInputNode).linkOutputNode = AST::NoId;
				}
				lastInputNode = inputNode;
				return;
			}
			// Pin didnt exist on the graph
			outputsComp.linkPins.Add(outputPin);
			outputsComp.linkInputNodes.Add(inputNode);
		}
	}

	void Connect(Tree& ast, AST::Id outputPin, AST::Id inputPin)
	{
		const Id outputNode = Hierarchy::GetParent(ast, outputPin);
		const Id inputNode  = Hierarchy::GetParent(ast, inputPin);
		Connect(ast, outputNode, outputPin, inputNode);
	}

	bool Disconnect(Tree& ast, Id linkId)
	{
		if (IsNone(linkId))
		{
			return false;
		}

		const Id inputNode = linkId;
		auto* inputComp    = ast.TryGet<CStatementInput>(inputNode);
		if (EnsureMsg(inputComp && !IsNone(inputComp->linkOutputNode),
		        "Trying to disconnect a unexistant link")) [[likely]]
		{
			// We expect the other side to have outputs component
			auto& outputsComp = ast.Get<CStatementOutputs>(inputComp->linkOutputNode);
			if (Id* lastInputNode = outputsComp.linkInputNodes.Find(inputNode)) [[likely]]
			{
				*lastInputNode = AST::NoId;
			}
			inputComp->linkOutputNode = AST::NoId;
			return true;
		}
		return false;
	}

	bool DisconnectFromInputPin(Tree& ast, AST::Id inputPin)
	{
		return Disconnect(ast, Hierarchy::GetParent(ast, inputPin));
	}
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin)
	{
		return DisconnectFromOutputPin(ast, outputPin, Hierarchy::GetParent(ast, outputPin));
	}
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin, AST::Id outputNode)
	{
		// NOTE: Can be optimized if needed since outputs is accessed twice counting Disconnect()
		if (auto* outputsComp = ast.TryGet<CStatementOutputs>(outputNode))
		{
			i32 pinIndex = outputsComp->linkPins.FindIndex(outputPin);
			if (pinIndex != NO_INDEX) [[likely]]
			{
				return Disconnect(ast, outputsComp->linkInputNodes[pinIndex]);
			}
		}
		return false;
	}
}    // namespace Rift::AST::StatementGraph
