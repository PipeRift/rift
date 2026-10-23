// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CModule.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Statements.h"
#include "AST/Components/Tags/CChanged.h"

#include <PipeECS.h>


namespace rift::ast
{
	struct Tree;
}

namespace rift::ast::TypeSystem
{
	void Init(Tree& ast);

	void SyncTypesByPath(p::TIdScopeRef<CDeclType, CFileRef> scope);

	using PropagateTypesScope = p::TIdScopeRef<p::Writes<CExprTypeId, CStmtAssign>, CExprDeclRefId,
	    CDeclVariable, CExprInputs, CExprOutputs, CExprUnaryOperator, CExprBinaryOperator,
	    CDeclType, CChanged, p::CParent>;
	// Propagates the type of every expression from the variable it references or assigns and
	// from the expressions it is connected to. Operators pass on the first type they receive.
	void PropagateTypes(PropagateTypesScope scope);

	void ResolveExprTypeIds(p::TIdScopeRef<p::Writes<CExprTypeId>, CExprType, CNamespace,
	    p::CParent, p::CChild, CModule, CDeclNative>
	        scope);
}    // namespace rift::ast::TypeSystem
