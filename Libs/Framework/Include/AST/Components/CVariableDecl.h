// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"
#include "Reflection/ReflectionFlags.h"


namespace Rift
{
	struct CVariableDecl : public CDeclaration
	{
		STRUCT(CVariableDecl, CDeclaration)

		PROP(typeId, Prop_NotSerialized)
		AST::Id typeId = AST::NoId;
	};
}    // namespace Rift
