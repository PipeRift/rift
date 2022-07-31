// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprType.h"

#include <Pipe/ECS/Access.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::TypeSystem
{
	using namespace p;


	void Init(AST::Tree& ast);

	using PropagateVariableTypesAccess =
	    TAccessRef<CExprDeclRefId, CDeclVariable, TWrite<CExprTypeId>>;
	void PropagateVariableTypes(PropagateVariableTypesAccess access);
	void PropagateExpressionTypes(AST::Tree& ast);

	void ResolveExprTypeIds(
	    TAccessRef<TWrite<CExprTypeId>, CExprType, CNamespace, CParent, CChild> access);
}    // namespace rift::TypeSystem
