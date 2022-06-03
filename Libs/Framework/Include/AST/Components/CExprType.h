// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprType : public Struct
	{
		STRUCT(CExprType, Struct)

		PROP(id, Prop_NotSerialized)
		AST::Id id = AST::NoId;


		CExprType(AST::Id id = AST::NoId) : id{id} {}
	};
}    // namespace Rift
