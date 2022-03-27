// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"


namespace Rift
{
	struct CDeclParameter : public CDeclaration
	{
		STRUCT(CDeclParameter, CDeclaration)

		PROP(typeId, Prop_NotSerialized)
		AST::Id typeId = AST::NoId;

		PROP(type)
		Name type;
	};
}    // namespace Rift
