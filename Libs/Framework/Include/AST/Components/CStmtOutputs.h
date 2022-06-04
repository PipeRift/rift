// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>
#include <Serialization/Contexts.h>


namespace Rift
{
	using namespace Pipe;


	struct CStmtOutput : public Pipe::Struct
	{
		STRUCT(CStmtOutput, Pipe::Struct)

		PROP(linkInputNode)
		AST::Id linkInputNode = AST::NoId;
	};


	struct CStmtOutputs : public Pipe::Struct
	{
		STRUCT(CStmtOutputs, Pipe::Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(pinIds)
		TArray<AST::Id> pinIds;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(Pipe::TArray<AST::Id> pins)
		    : pinIds{Move(pins)}, linkInputNodes(pinIds.Size(), AST::NoId)
		{}
	};

	static void Read(ReadContext& ct, CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
	static void Write(WriteContext& ct, const CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
}    // namespace Rift
