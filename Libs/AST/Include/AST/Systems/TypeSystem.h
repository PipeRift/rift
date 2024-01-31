// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CChanged.h"

#include <PipeECS.h>


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

	using PropagateExpressionTypesAccess = TAccessRef<CDeclType, CChanged, CExprInputs,
	    CExprOutputs, TWrite<CExprTypeId>, CExprUnaryOperator, CExprBinaryOperator, CParent>;
	void PropagateExpressionTypes(PropagateExpressionTypesAccess access);

	void ResolveExprTypeIds(
	    TAccessRef<TWrite<CExprTypeId>, CExprType, CNamespace, CParent, CChild> access);
}    // namespace rift::AST::TypeSystem
