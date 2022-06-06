// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"
#include "AST/Id.h"
#include "PRefl/ReflectionFlags.h"


namespace rift
{
	struct CDeclVariable : public CDeclaration
	{
		STRUCT(CDeclVariable, CDeclaration)

		PROP(typeId, p::Prop_NotSerialized)
		AST::Id typeId = AST::NoId;
	};
}    // namespace rift
