// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CNamespace.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Id.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"
#include "AST/Utils/Namespaces.h"
#include "AST/Utils/TypeUtils.h"

#include <PipeECS.h>


namespace rift::ast::TypeSystem
{
	void Init(Tree& ast)
	{
		p::TIdScope<CDeclType, CNamespace> scope{ast};
	}

	void SyncTypesByPath(p::TIdScopeRef<CDeclType, CFileRef> scope)
	{
		if (scope.PoolSize<p::CMdfd<CFileRef>>() <= 0)
		{
			return;
		}

		auto& types = scope.GetOrSetStatic<STypes>();
		for (Id id : p::FindAllIdsWith<p::CMdfd<CFileRef>>(scope))
		{
			const auto& file = scope.Get<const CFileRef>(id);
			types.typesByPath.Insert(p::Tag{file.path}, id);
		}
	}

	void PropagateVariableTypes(PropagateVariableTypesScope scope)
	{
		for (Id id : FindAllIdsWith<CExprDeclRefId>(scope))
		{
			const Id declId = scope.Get<const CExprDeclRefId>(id).declarationId;
			if (scope.IsValid(declId))
			{
				const Id typeId = scope.Get<const CDeclVariable>(declId).typeId;
				scope.Add<CExprTypeId>(id, {.id = typeId});
			}
		}
	}

	bool PropagateUnaryOperator(p::TIdScope<p::Writes<CExprTypeId>, CExprInputs> scope, Id nodeId)
	{
		const Id outputId  = nodeId;    // Output in unary operator is same as the node itself
		const auto& inputs = scope.Get<const CExprInputs>(nodeId);
		const ExprOutput* linkedOutputId = inputs.linkedOutputs.At(0);
		if (linkedOutputId && linkedOutputId->pinId != NoId)
		{
			return CopyExpressionType(scope, linkedOutputId->pinId, outputId);
		}
		return false;
	}

	bool PropagateBinaryOperator(p::TIdScope<p::Writes<CExprTypeId>, CExprInputs> scope, Id nodeId)
	{
		const auto& inputs = scope.Get<const CExprInputs>(nodeId);
		Id outputId        = nodeId;    // Output in binary operator is same as the node itself
		if (inputs.pinIds.Size() == 2) [[likely]]
		{
			const ExprOutput firstLinkedOutputId = inputs.linkedOutputs[0];
			// Set input types
			CopyExpressionType(scope, firstLinkedOutputId.pinId, inputs.pinIds[0]);
			CopyExpressionType(scope, inputs.linkedOutputs[1].pinId, inputs.pinIds[1]);
			// Set output type from first connection
			return CopyExpressionType(scope, firstLinkedOutputId.pinId, outputId);
		}
		return false;
	}

	void PropagateExpressionTypes(PropagateExpressionTypesScope scope)
	{
		p::TArray<Id> dirtyTypeIds = p::FindAllIdsWith<CDeclType, CChanged>(scope);

		p::TArray<Id> dirtyNodeIds;
		p::GetIdChildren(scope, dirtyTypeIds, dirtyNodeIds);

		// Make sure the nodes have inputs and outputs
		p::ExcludeIdsWithout<CExprInputs, CExprOutputs>(scope, dirtyNodeIds);

		// Only Unary or Binary operators propagate as of right now
		p::ExcludeIdsWithoutAny<CExprUnaryOperator, CExprBinaryOperator>(scope, dirtyNodeIds);

		bool anyPropagated;
		while (!dirtyNodeIds.IsEmpty())    // Repeat until nothing to propagate
		{
			bool anyPropagated = false;
			// Propagate all dirty nodes, remove successfully propagated ones
			for (p::i32 i = dirtyNodeIds.Size() - 1; i >= 0; --i)
			{
				const Id nodeId = dirtyNodeIds[i];

				if (scope.Has<CExprUnaryOperator>(nodeId))
				{
					if (PropagateUnaryOperator(scope, nodeId))
					{
						dirtyNodeIds.RemoveAtSwapUnsafe(i);
						anyPropagated = true;
					}
				}
				else if (scope.Has<CExprBinaryOperator>(nodeId))
				{
					if (PropagateBinaryOperator(scope, nodeId))
					{
						dirtyNodeIds.RemoveAtSwapUnsafe(i);
						anyPropagated = true;
					}
				}
			}

			if (!anyPropagated)    // If nothing propagated, there is nothing else we can do
			{
				break;
			}
		}
	}

	void ResolveExprTypeIds(
	    p::TIdScopeRef<p::Writes<CExprTypeId>, CExprType, CNamespace, CParent, CChild> scope)
	{
		auto callExprs = p::FindAllIdsWith<CExprType>(scope);
		p::ExcludeIdsWith<CExprTypeId>(scope, callExprs);
		for (Id id : callExprs)
		{
			auto& expr      = scope.Get<const CExprType>(id);
			const Id typeId = FindIdFromNamespace(scope, expr.type);
			if (!p::IsNone(typeId))
			{
				scope.Add(id, CExprTypeId{.id = typeId, .mode = expr.mode});
			}
		}
	}
}    // namespace rift::ast::TypeSystem
