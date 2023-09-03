// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"
#include "AST/Id.h"

#include <Pipe/Reflect/ReflectionFlags.h>


namespace rift::AST
{
	struct CDeclVariable : public CDeclaration
	{
		P_STRUCT(CDeclVariable, CDeclaration)

		P_PROP(typeId, p::Prop_NotSerialized)
		Id typeId = NoId;
	};
}    // namespace rift::AST
