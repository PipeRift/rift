// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"
#include "Reflection/ReflectionFlags.h"


namespace Rift
{
	struct CVariableDecl : public CDeclaration
	{
		STRUCT(CVariableDecl, CDeclaration)

		PROP(typeId, Prop_Transient)
		AST::Id typeId = AST::NoId;
	};
}    // namespace Rift
