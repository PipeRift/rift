// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Tags/CChanged.h"

#include <PipeECS.h>


namespace rift::ast
{
	struct Tree;
}

namespace rift::ast::TypeSystem
{
	void Init(Tree& ast);

	using PropagateVariableTypesAccess =
	    p::TAccessRef<CExprDeclRefId, CDeclVariable, p::TWrite<CExprTypeId>>;
	void PropagateVariableTypes(PropagateVariableTypesAccess access);

	using PropagateExpressionTypesAccess = p::TAccessRef<CDeclType, CChanged, CExprInputs,
	    CExprOutputs, p::TWrite<CExprTypeId>, CExprUnaryOperator, CExprBinaryOperator, p::CParent>;
	void PropagateExpressionTypes(PropagateExpressionTypesAccess access);

	void ResolveExprTypeIds(
	    p::TAccessRef<p::TWrite<CExprTypeId>, CExprType, CNamespace, p::CParent, p::CChild> access);
}    // namespace rift::ast::TypeSystem
