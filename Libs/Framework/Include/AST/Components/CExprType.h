// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Reflection/Struct.h>


namespace Rift
{
	struct CExprType : public Pipe::Struct
	{
		STRUCT(CExprType, Pipe::Struct)

		PROP(id, Pipe::Prop_NotSerialized)
		AST::Id id = AST::NoId;


		CExprType(AST::Id id = AST::NoId) : id{id} {}
	};
}    // namespace Rift
