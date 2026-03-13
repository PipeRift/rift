// Copyright 2015-2026 Piperift. All Rights Reserved.
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

	using PropagateVariableTypesScope =
	    p::TIdScopeRef<p::Writes<CExprTypeId>, CExprDeclRefId, CDeclVariable>;
	void PropagateVariableTypes(PropagateVariableTypesScope scope);

	using PropagateExpressionTypesScope = p::TIdScopeRef<p::Writes<CExprTypeId>, CDeclType,
	    CChanged, CExprInputs, CExprOutputs, CExprUnaryOperator, CExprBinaryOperator, p::CParent>;
	void PropagateExpressionTypes(PropagateExpressionTypesScope scope);

	void ResolveExprTypeIds(
	    p::TIdScopeRef<p::Writes<CExprTypeId>, CExprType, CNamespace, p::CParent, p::CChild> scope);
}    // namespace rift::ast::TypeSystem
