// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Types.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CChild : public Struct
	{
		STRUCT(CChild, Struct)

		PROP(parent)
		AST::Id parent = AST::NoId;


		CChild() = default;
		CChild(AST::Id parent) : parent(parent) {}
	};
}    // namespace Rift
