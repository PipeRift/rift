// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Statements.h"

#include "AST/Id.h"

#include <Pipe/ECS/Utils/Hierarchy.h>


namespace rift::AST::Statements
{
	bool CanConnect(const Tree& ast, Id outputNode, Id outputPin, Id inputNode)
	{
		if (outputNode == inputNode)
		{
			return false;
		}

		if ((!ast.Has<CStmtOutput>(outputNode) && !ast.Has<CStmtOutputs>(outputNode))
		    || !ast.Has<CStmtInput>(inputNode))
		{
			return false;
		}

		return !WouldLoop(ast, outputNode, outputPin, inputNode);
	}

	bool TryConnect(Tree& ast, Id outputPin, Id inputNode)
	{
		if (!Ensure(!IsNone(outputPin) && !IsNone(inputNode)))
		{
			return false;
		}

		// Resolve output node. Sometimes the output pin itself is the node
		Id outputNode = AST::NoId;
		if (ast.Has<CStmtOutput>(outputPin))
		{
			outputNode = outputPin;
		}
		else
		{
			outputNode = p::ecs::GetParent(ast, outputPin);
			if (IsNone(outputNode))
			{
				return false;
			}
		}

		if (!CanConnect(ast, outputNode, outputPin, inputNode))
		{
			return false;
		}


		auto& inputComp = ast.Get<CStmtInput>(inputNode);
		if (inputComp.linkOutputNode != AST::NoId)
		{
			// Disconnect previous output connected to input if any
			if (auto* lastOutputComp = ast.TryGet<CStmtOutput>(inputComp.linkOutputNode))
			{
				lastOutputComp->linkInputNode = AST::NoId;
			}
			else if (auto* lastOutputsComp = ast.TryGet<CStmtOutputs>(inputComp.linkOutputNode))
			{
				lastOutputsComp->linkInputNodes.FindRef(inputNode) = AST::NoId;
			}
		}
		inputComp.linkOutputNode = outputNode;


		if (auto* outputComp = ast.TryGet<CStmtOutput>(outputNode))
		{
			// Connect if single output
			Id& lastInputNode = outputComp->linkInputNode;
			// Disconnect previous input connected to output if any
			if (lastInputNode != AST::NoId && ast.Has<CStmtInput>(lastInputNode))
			{
				ast.Get<CStmtInput>(lastInputNode).linkOutputNode = AST::NoId;
			}
			lastInputNode = inputNode;
		}
		else if (auto* outputsComp = ast.TryGet<CStmtOutputs>(outputNode))
		{
			// Connect if multiple output
			const i32 pinIndex = outputsComp->pinIds.FindIndex(outputPin);
			if (pinIndex != NO_INDEX)
			{
				Id& lastInputNode = outputsComp->linkInputNodes[pinIndex];
				// Disconnect previous input connected to output if any
				if (lastInputNode != AST::NoId && ast.Has<CStmtInput>(lastInputNode))
				{
					ast.Get<CStmtInput>(lastInputNode).linkOutputNode = AST::NoId;
				}
				lastInputNode = inputNode;
			}
			else
			{
				// Pin didn't exist on the graph
				outputsComp->pinIds.Add(outputPin);
				outputsComp->linkInputNodes.Add(inputNode);
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

		// Input node is always the same id as linkId
		auto* inputComp = ast.TryGet<CStmtInput>(linkId);
		if (inputComp
		    && EnsureMsg(!IsNone(inputComp->linkOutputNode),
		        "Trying to disconnect a unexistant link")) [[likely]]
		{
			// We expect the other side to have outputs component
			auto& outputsComp = ast.Get<CStmtOutputs>(inputComp->linkOutputNode);
			if (Id* lastInputNode = outputsComp.linkInputNodes.Find(linkId)) [[likely]]
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
		// NOTE: Input pin ids equal input node ids
		return Disconnect(ast, inputPin);
	}
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin)
	{
		return DisconnectFromOutputPin(ast, outputPin, p::ecs::GetParent(ast, outputPin));
	}
	bool DisconnectFromOutputPin(Tree& ast, AST::Id outputPin, AST::Id outputNode)
	{
		// NOTE: Can be optimized if needed since outputs is accessed twice counting
		// Disconnect()
		if (auto* outputsComp = ast.TryGet<CStmtOutputs>(outputNode))
		{
			i32 pinIndex = outputsComp->pinIds.FindIndex(outputPin);
			if (pinIndex != NO_INDEX) [[likely]]
			{
				return Disconnect(ast, outputsComp->linkInputNodes[pinIndex]);
			}
		}
		return false;
	}

	bool WouldLoop(const Tree& ast, Id outputNode, Id outputPin, Id inputNode)
	{
		AST::Id currentNode = outputNode;
		while (!IsNone(currentNode))
		{
			const auto* input = ast.TryGet<CStmtInput>(currentNode);
			if (!input)
			{
				return false;
			}
			else if (input->linkOutputNode == inputNode)
			{
				return true;
			}
			currentNode = input->linkOutputNode;
		}
		return false;
	}

	Id GetConnectedToInput(TAccessRef<CStmtInput> access, Id node)
	{
		if (const auto* input = access.TryGet<const CStmtInput>(node))
		{
			return input->linkOutputNode;
		}
		return NoId;
	}

	void GetConnectedToInputs(TAccessRef<CStmtInput> access, TSpan<const Id> nodes, TArray<Id>& ids)
	{
		for (const Id node : nodes)
		{
			if (const auto* input = access.TryGet<const CStmtInput>(node))
			{
				ids.Add(input->linkOutputNode);
			}
		}
	}

	TSpan<Id> GetConnectedToOutputs(TAccessRef<CStmtOutputs> access, Id node)
	{
		if (const auto* output = access.TryGet<const CStmtOutputs>(node))
		{
			return output->linkInputNodes;
		}
		return {};
	}

	void GetConnectedToOutputs(
	    TAccessRef<CStmtOutputs> access, TSpan<const Id> nodes, TArray<Id>& ids)
	{
		for (const Id node : nodes)
		{
			if (const auto* output = access.TryGet<const CStmtOutputs>(node))
			{
				ids.Append(output->linkInputNodes);
			}
		}
	}

	void GetChain(TAccessRef<CStmtOutput, CStmtOutputs> access, Id firstStmtId, TArray<Id>& stmtIds,
	    Id& splitStmtId)
	{
		Id id = firstStmtId;
		while (id != AST::NoId && access.Has<CStmtOutput>(id))
		{
			stmtIds.Add(id);
			id = access.Get<const CStmtOutput>(id).linkInputNode;
		}

		if (access.Has<CStmtOutputs>(id))
		{
			splitStmtId = id;
		}
	}
}    // namespace rift::AST::Statements
