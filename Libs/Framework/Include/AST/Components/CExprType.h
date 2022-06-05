// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace rift
{
	struct CExprType : public pipe::Struct
	{
		STRUCT(CExprType, pipe::Struct)

		PROP(id, pipe::Prop_NotSerialized)
		AST::Id id = AST::NoId;


		CExprType(AST::Id id = AST::NoId) : id{id} {}
	};
}    // namespace rift
