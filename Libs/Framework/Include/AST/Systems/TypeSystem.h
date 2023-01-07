// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"

#include <Pipe/ECS/Access.h>


namespace rift::AST
{
	struct Tree;
}

namespace rift::AST::TypeSystem
{
	using namespace p;


	void Init(Tree& ast);

	using PropagateVariableTypesAccess =
	    TAccessRef<CExprDeclRefId, CDeclVariable, TWrite<CExprTypeId>>;
	void PropagateVariableTypes(PropagateVariableTypesAccess access);

	using PropagateExpressionTypesAccess = TAccessRef<CType, CChanged, CExprInputs, CExprOutputs,
	    TWrite<CExprTypeId>, CExprUnaryOperator, CExprBinaryOperator, CParent>;
	void PropagateExpressionTypes(PropagateExpressionTypesAccess access);

	void ResolveExprTypeIds(
	    TAccessRef<TWrite<CExprTypeId>, CExprType, CNamespace, CParent, CChild> access);
}    // namespace rift::AST::TypeSystem
