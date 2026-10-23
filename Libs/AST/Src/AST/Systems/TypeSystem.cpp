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
	void Init(Tree& ast) {}

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

	namespace
	{
		bool PropagateUnaryOperator(
		    p::TIdScope<p::Writes<CExprTypeId>, CExprInputs> scope, Id nodeId)
		{
			const Id outputId  = nodeId;    // Output in unary operator is same as the node itself
			const auto& inputs = scope.Get<const CExprInputs>(nodeId);
			const ExprOutput* linkedOutput = inputs.linkedOutputs.At(0);
			if (linkedOutput && linkedOutput->pinId != NoId)
			{
				return CopyExpressionType(scope, linkedOutput->pinId, outputId);
			}
			return false;
		}

		bool PropagateBinaryOperator(
		    p::TIdScope<p::Writes<CExprTypeId>, CExprInputs> scope, Id nodeId)
		{
			const auto& inputs = scope.Get<const CExprInputs>(nodeId);
			Id outputId        = nodeId;    // Output in binary operator is same as the node itself
			if (inputs.pinIds.Size() == 2) [[likely]]
			{
				const ExprOutput firstLinkedOutput = inputs.linkedOutputs[0];
				CopyExpressionType(scope, firstLinkedOutput.pinId, inputs.pinIds[0]);
				CopyExpressionType(scope, inputs.linkedOutputs[1].pinId, inputs.pinIds[1]);
				return CopyExpressionType(scope, firstLinkedOutput.pinId, outputId);
			}
			return false;
		}
	}    // namespace

	void PropagateTypes(PropagateTypesScope scope)
	{
		// References and assignments share the type of their variable
		for (Id id : FindAllIdsWith<CExprDeclRefId>(scope))
		{
			const Id declId = scope.Get<const CExprDeclRefId>(id).declarationId;
			if (const auto* variable = scope.TryGet<const CDeclVariable>(declId))
			{
				scope.Add<CExprTypeId>(id, {.id = variable->typeId});
			}
		}

		for (Id id : FindAllIdsWith<CStmtAssign>(scope))
		{
			const auto* variable =
			    scope.TryGet<const CDeclVariable>(scope.Get<const CStmtAssign>(id).declarationId);
			if (!variable)
			{
				continue;
			}

			const auto* inputs  = scope.TryGet<const CExprInputs>(id);
			const auto* outputs = scope.TryGet<const CExprOutputs>(id);
			if (!inputs || !outputs || inputs->pinIds.IsEmpty() || outputs->pinIds.IsEmpty())
			{
				continue;
			}

			const Id typeId = variable->typeId;
			scope.Add<CExprTypeId>(inputs->pinIds[0], {.id = typeId});
			scope.Add<CExprTypeId>(outputs->pinIds[0], {.id = typeId});
		}

		// Operators take the type of their first input, so keep propagating until none changes
		p::TArray<Id> dirtyTypeIds = p::FindAllIdsWith<CDeclType, CChanged>(scope);

		p::TArray<Id> dirtyNodeIds;
		p::GetIdChildren(scope, dirtyTypeIds, dirtyNodeIds);

		p::ExcludeIdsWithout<CExprInputs, CExprOutputs>(scope, dirtyNodeIds);

		// Only unary and binary operators propagate
		p::ExcludeIdsWithoutAny<CExprUnaryOperator, CExprBinaryOperator>(scope, dirtyNodeIds);

		while (!dirtyNodeIds.IsEmpty())
		{
			bool anyPropagated = false;
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

			if (!anyPropagated)
			{
				break;
			}
		}
	}

	void ResolveExprTypeIds(p::TIdScopeRef<p::Writes<CExprTypeId>, CExprType, CNamespace, CParent,
	    CChild, CModule, CDeclNative>
	        scope)
	{
		auto typeExprs = p::FindAllIdsWith<CExprType>(scope);
		p::ExcludeIdsWith<CExprTypeId>(scope, typeExprs);
		for (Id id : typeExprs)
		{
			const auto& expr = scope.Get<const CExprType>(id);
			const Id typeId  = FindIdFromNamespace(scope, expr.type);
			if (!p::IsNone(typeId))
			{
				scope.Add(id, CExprTypeId{.id = typeId, .mode = expr.mode});
			}
		}
	}
}    // namespace rift::ast::TypeSystem
