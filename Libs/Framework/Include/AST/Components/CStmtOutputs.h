// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <PRefl/Struct.h>
#include <PSerl/Serialization.h>


namespace rift
{
	using namespace p;


	struct CStmtOutput : public p::Struct
	{
		STRUCT(CStmtOutput, p::Struct)

		PROP(linkInputNode)
		AST::Id linkInputNode = AST::NoId;
	};


	struct CStmtOutputs : public p::Struct
	{
		STRUCT(CStmtOutputs, p::Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(pinIds)
		TArray<AST::Id> pinIds;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(p::TArray<AST::Id> pins)
		    : pinIds{Move(pins)}, linkInputNodes(pinIds.Size(), AST::NoId)
		{}
	};

	static void Read(Reader& ct, CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
	static void Write(Writer& ct, const CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
}    // namespace rift
