// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Containers/Set.h>
#include <Lang/AST.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CChildren : public Struct
	{
		STRUCT(CChildren, Struct)

		PROP(TArray<AST::Id>, children)
		TArray<AST::Id> children;
	};
}    // namespace Rift
