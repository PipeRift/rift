// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/StatementGraph.h"

#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"



namespace Rift::AST::StatementGraph
{
	bool InternalDisconnectOutput(Tree& ast, AST::Id inputNode, AST::Id outputNode)
	{
		if (ast.IsValid(outputNode))
		{
			if (auto* outputsComp = ast.TryGet<CStatementOutputs>(outputNode))
			{
				outputsComp->edgeInputNodes.Remove(inputNode);
				if (outputsComp->edgeInputNodes.IsEmpty())
				{
					ast.Remove<CStatementOutputs>(outputNode);
				}
				return true;
			}
		}
		return false;
	}

	void Connect(Tree& ast, AST::Id outputNode, AST::Id outputPin, AST::Id inputNode)
	{
		Check(!IsNone(outputNode) && !IsNone(outputPin) && !IsNone(inputNode));

		auto& outputsComp = ast.GetOrAdd<CStatementOutputs>(outputNode);
		auto& inputComp   = ast.GetOrAdd<CStatementInput>(inputNode);

		// Disconnect previous output if any
		InternalDisconnectOutput(ast, inputNode, inputComp.edgeOutputNode);
		outputsComp.edgeInputNodes.AddUnique(inputNode);
		inputComp.edgeOutputNode = outputNode;
		inputComp.edgeOutputPin  = outputPin;
	}

	bool Disconnect(Tree& ast, AST::Id edgeId)
	{
		const AST::Id inputNode = edgeId;
		const auto& inputComp   = ast.Get<CStatementInput>(inputNode);
		if (!InternalDisconnectOutput(ast, inputNode, inputComp.edgeOutputNode))
		{
			ast.Remove<CStatementInput>(inputNode);
			return true;
		}
		return false;
	}

	void DisconnectAllInputs(Tree& ast, TSpan<const AST::Id> outputs) {}
}    // namespace Rift::AST::StatementGraph
