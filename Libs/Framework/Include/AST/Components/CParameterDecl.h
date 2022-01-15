// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"


namespace Rift
{
	struct CParameterDecl : public CDeclaration
	{
		STRUCT(CParameterDecl, CDeclaration)

		PROP(typeId, Prop_NotSerialized)
		AST::Id typeId = AST::NoId;

		PROP(type)
		Name type;
	};
}    // namespace Rift
