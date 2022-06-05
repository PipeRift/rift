// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>
#include <Serialization/Contexts.h>


namespace rift
{
	using namespace pipe;


	struct CStmtOutput : public pipe::Struct
	{
		STRUCT(CStmtOutput, pipe::Struct)

		PROP(linkInputNode)
		AST::Id linkInputNode = AST::NoId;
	};


	struct CStmtOutputs : public pipe::Struct
	{
		STRUCT(CStmtOutputs, pipe::Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(pinIds)
		TArray<AST::Id> pinIds;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(pipe::TArray<AST::Id> pins)
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
}    // namespace rift
