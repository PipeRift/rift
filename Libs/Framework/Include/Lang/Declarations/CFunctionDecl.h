// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/CDeclaration.h"

#include <Lang/AST.h>


namespace Rift
{
	struct CFunctionDecl : public CDeclaration
	{
		STRUCT(CFunctionDecl, CDeclaration)

		// AST nodes representing function parameters
		TArray<AST::Id> parameterNodes;

		// Ordered list of AST nodes that constitute the function's body
		TArray<AST::Id> bodyNodes;

		TOptional<AST::Id> returnNode;
	};
}    // namespace Rift
