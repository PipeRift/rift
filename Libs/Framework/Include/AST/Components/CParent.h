// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CParent : public Struct
	{
		STRUCT(CParent, Struct)

		PROP(AST::Id, parent)
		AST::Id parent = AST::NoId;
	};
}    // namespace Rift
