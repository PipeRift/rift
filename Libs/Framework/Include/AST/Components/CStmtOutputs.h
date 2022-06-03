// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Serialization/Contexts.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CStmtOutput : public Struct
	{
		STRUCT(CStmtOutput, Struct)

		PROP(linkInputNode)
		AST::Id linkInputNode = AST::NoId;
	};

	static void Read(Serl::ReadContext& ct, CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
	static void Write(Serl::WriteContext& ct, const CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}


	struct CStmtOutputs : public Struct
	{
		STRUCT(CStmtOutputs, Struct)

		// Both arrays keep the same index to the input node and the output pin
		PROP(pinIds)
		TArray<AST::Id> pinIds;
		PROP(linkInputNodes)
		TArray<AST::Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(TArray<AST::Id> pins)
		    : pinIds{Move(pins)}, linkInputNodes(pinIds.Size(), AST::NoId)
		{}
	};
}    // namespace Rift
