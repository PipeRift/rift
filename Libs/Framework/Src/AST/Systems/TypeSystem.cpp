// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"
#include "AST/Utils/TypeUtils.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/ECS/Utils/Hierarchy.h>


namespace rift::AST::TypeSystem
{
	void Init(Tree& ast)
	{
		TAccess<CDeclType, CNamespace> access{ast};

		ast.OnAdd<CFileRef>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (Id id : ids)
			{
				if (ast.template Has<CDeclType>(id) && ast.template Has<CFileRef>(id))
				{
					const auto& file = ast.template Get<const CFileRef>(id);
					types.typesByPath.Insert(p::Name{file.path}, id);
				}
			}
		});

		ast.OnRemove<CFileRef>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (Id id : ids)
			{
				if (ast.template Has<CDeclType>(id) && ast.template Has<CFileRef>(id))
				{
					const auto& file = ast.template Get<const CFileRef>(id);
					types.typesByPath.Remove(p::Name{file.path});
				}
			}
		});
	}

	void PropagateVariableTypes(PropagateVariableTypesAccess access)
	{
		for (Id id : ecs::ListAll<CExprDeclRefId>(access))
		{
			const Id declId = access.Get<const CExprDeclRefId>(id).declarationId;
			if (access.IsValid(declId))
			{
				const Id typeId = access.Get<const CDeclVariable>(declId).typeId;
				access.Add<CExprTypeId>(id, {.id = typeId});
			}
		}
	}

	bool PropagateUnaryOperator(TAccess<CExprInputs, TWrite<CExprTypeId>> access, Id nodeId)
	{
		const Id outputId  = nodeId;    // Output in unary operator is same as the node itself
		const auto& inputs = access.Get<const CExprInputs>(nodeId);
		const ExprOutput* linkedOutputId = inputs.linkedOutputs.At(0);
		if (linkedOutputId && linkedOutputId->pinId != NoId)
		{
			return CopyExpressionType(access, linkedOutputId->pinId, outputId);
		}
		return false;
	}

	bool PropagateBinaryOperator(TAccess<CExprInputs, TWrite<CExprTypeId>> access, Id nodeId)
	{
		const auto& inputs = access.Get<const CExprInputs>(nodeId);
		Id outputId        = nodeId;    // Output in binary operator is same as the node itself
		if (inputs.pinIds.Size() == 2) [[likely]]
		{
			const ExprOutput firstLinkedOutputId = inputs.linkedOutputs[0];
			// Set input types
			CopyExpressionType(access, firstLinkedOutputId.pinId, inputs.pinIds[0]);
			CopyExpressionType(access, inputs.linkedOutputs[1].pinId, inputs.pinIds[1]);
			// Set output type from first connection
			return CopyExpressionType(access, firstLinkedOutputId.pinId, outputId);
		}
		return false;
	}

	void PropagateExpressionTypes(PropagateExpressionTypesAccess access)
	{
		TArray<Id> dirtyTypeIds = ecs::ListAll<CDeclType, CChanged>(access);

		TArray<Id> dirtyNodeIds;
		p::ecs::GetChildren(access, dirtyTypeIds, dirtyNodeIds);

		// Make sure the nodes have inputs and outputs
		ecs::ExcludeIfNot<CExprInputs, CExprOutputs>(access, dirtyNodeIds);

		// Only Unary and Binary operators propagate as of right now
		ecs::ExcludeIf(dirtyNodeIds, [&access](Id id) {
			return !access.Has<CExprUnaryOperator>(id) && !access.Has<CExprBinaryOperator>(id);
		});

		bool anyPropagated;
		while (!dirtyNodeIds.IsEmpty())    // Repeat until nothing to propagate
		{
			bool anyPropagated = false;
			// Propagate all dirty nodes, remove successfully propagated ones
			for (i32 i = dirtyNodeIds.Size() - 1; i >= 0; --i)
			{
				const Id nodeId = dirtyNodeIds[i];

				if (access.Has<CExprUnaryOperator>(nodeId))
				{
					if (PropagateUnaryOperator(access, nodeId))
					{
						dirtyNodeIds.RemoveAtSwapUnsafe(i);
						anyPropagated = true;
					}
				}
				else if (access.Has<CExprBinaryOperator>(nodeId))
				{
					if (PropagateBinaryOperator(access, nodeId))
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
	    TAccessRef<TWrite<CExprTypeId>, CExprType, CNamespace, CParent, CChild> access)
	{
		auto callExprs = ecs::ListAll<CExprType>(access);
		ecs::ExcludeIf<CExprTypeId>(access, callExprs);
		for (Id id : callExprs)
		{
			auto& expr      = access.Get<const CExprType>(id);
			const Id typeId = FindIdFromNamespace(access, expr.type);
			if (!IsNone(typeId))
			{
				access.Add(id, CExprTypeId{.id = typeId, .mode = expr.mode});
			}
		}
	}
}    // namespace rift::AST::TypeSystem
