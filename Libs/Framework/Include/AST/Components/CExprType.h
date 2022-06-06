// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <PRefl/Struct.h>


namespace rift
{
	struct CExprType : public p::Struct
	{
		STRUCT(CExprType, p::Struct)

		PROP(id, p::Prop_NotSerialized)
		AST::Id id = AST::NoId;


		CExprType(AST::Id id = AST::NoId) : id{id} {}
	};
}    // namespace rift
