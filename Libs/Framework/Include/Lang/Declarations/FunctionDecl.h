// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declaration.h"

#include <ECS.h>


namespace Rift
{
	struct CFunctionDecl : public Declaration
	{
		STRUCT(CFunctionDecl, Declaration)

		// AST nodes representing function parameters
		TArray<ECS::EntityId> parameterNodes;

		// Ordered list of AST nodes that constitute the function's body
		TArray<ECS::EntityId> bodyNodes;

		TOptional<ECS::EntityId> returnNode;
	};
}    // namespace Rift
