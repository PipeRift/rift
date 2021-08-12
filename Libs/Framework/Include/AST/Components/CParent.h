// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Types.h>
#include <Containers/Array.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CParent : public Struct
	{
		STRUCT(CParent, Struct)

		PROP(children)
		TArray<AST::Id> children;
	};
}    // namespace Rift
