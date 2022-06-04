// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace Rift
{
	using namespace Pipe;

	struct CStmtInput : public Struct
	{
		STRUCT(CStmtInput, Struct)

		PROP(linkOutputNode)
		AST::Id linkOutputNode = AST::NoId;
	};

	static void Read(ReadContext& ct, CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Write(WriteContext& ct, const CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
}    // namespace Rift
