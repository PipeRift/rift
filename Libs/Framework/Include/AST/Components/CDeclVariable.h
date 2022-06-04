// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"
#include "AST/Id.h"
#include "Reflection/ReflectionFlags.h"


namespace Rift
{
	struct CDeclVariable : public CDeclaration
	{
		STRUCT(CDeclVariable, CDeclaration)

		PROP(typeId, Pipe::Prop_NotSerialized)
		AST::Id typeId = AST::NoId;
	};
}    // namespace Rift
