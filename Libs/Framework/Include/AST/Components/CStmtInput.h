// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CStmtInput : public Struct
	{
		STRUCT(CStmtInput, Struct)

		PROP(linkOutputNode)
		AST::Id linkOutputNode = AST::NoId;
	};
	static void Read(Serl::ReadContext& ct, CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Write(Serl::WriteContext& ct, const CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
}    // namespace Rift
