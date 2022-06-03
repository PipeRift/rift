// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprType.h"

#include <ECS/Access.h>


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::TypeSystem
{
	void Init(AST::Tree& ast);

	using PropagateVariableTypesAccess =
	    TAccessRef<CExprDeclRefId, CDeclVariable, TWrite<CExprType>>;
	void PropagateVariableTypes(PropagateVariableTypesAccess access);
	void PropagateExpressionTypes(AST::Tree& ast);
}    // namespace Rift::TypeSystem
