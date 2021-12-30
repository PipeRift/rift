// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"


namespace Rift
{
	struct CParameterDecl : public CDeclaration
	{
		STRUCT(CParameterDecl, CDeclaration)

		PROP(typeId)
		AST::Id typeId = AST::NoId;

		PROP(type)
		Name type;

		PROP(name)
		String name;
	};
}    // namespace Rift
