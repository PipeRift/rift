// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/Reflect/Struct.h>


namespace rift
{
	using namespace p;

	struct CStmtInput : public Struct
	{
		STRUCT(CStmtInput, Struct)

		PROP(linkOutputNode)
		AST::Id linkOutputNode = AST::NoId;
	};

	static void Read(Reader& ct, CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Write(Writer& ct, const CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
}    // namespace rift
