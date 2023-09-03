// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>
#include <Pipe/Serialize/Serialization.h>


namespace rift::AST
{
	using namespace p::core;


	struct CStmtOutput : public p::Struct
	{
		P_STRUCT(CStmtOutput, p::Struct)

		P_PROP(linkInputNode)
		Id linkInputNode = NoId;
	};


	struct CStmtOutputs : public p::Struct
	{
		P_STRUCT(CStmtOutputs, p::Struct)

		// Both arrays keep the same index to the input node and the output pin
		P_PROP(pinIds)
		TArray<Id> pinIds;
		P_PROP(linkInputNodes)
		TArray<Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(p::TArray<Id> pins) : pinIds{Move(pins)}, linkInputNodes(pinIds.Size(), NoId)
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
}    // namespace rift::AST
