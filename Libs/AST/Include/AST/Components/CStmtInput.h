// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	using namespace p;

	struct CStmtInput : public Struct
	{
		P_STRUCT(CStmtInput, Struct)

		P_PROP(linkOutputNode)
		Id linkOutputNode = NoId;
	};

	static void Read(Reader& ct, CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Write(Writer& ct, const CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
}    // namespace rift::AST
