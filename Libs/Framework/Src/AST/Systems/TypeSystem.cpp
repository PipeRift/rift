// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/TypeUtils.h"

#include <Pipe/ECS/Filtering.h>


namespace rift::TypeSystem
{
	void Init(AST::Tree& ast)
	{
		TAccess<CType, CNamespace> access{ast};

		ast.OnAdd<CFileRef>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (ast.template Has<CType>(id) && ast.template Has<CFileRef>(id))
				{
					const auto& file = ast.template Get<const CFileRef>(id);
					const Name pathName{ToString(file.path)};
					types.typesByPath.Insert(pathName, id);
				}
			}
		});

		ast.OnRemove<CFileRef>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (ast.template Has<CType>(id) && ast.template Has<CFileRef>(id))
				{
					const auto& file = ast.template Get<const CFileRef>(id);
					const Name pathName{ToString(file.path)};
					types.typesByPath.Remove(pathName);
				}
			}
		});
	}

	void PropagateVariableTypes(PropagateVariableTypesAccess access)
	{
		for (AST::Id id : ecs::ListAll<CExprDeclRefId>(access))
		{
			const AST::Id declId = access.Get<const CExprDeclRefId>(id).declarationId;
			if (access.IsValid(declId))
			{
				const AST::Id typeId = access.Get<const CDeclVariable>(declId).typeId;
				access.Add<CExprTypeId>(id, {.id = typeId});
			}
		}
	}

	bool PropagateUnaryOperator(TAccess<CExprInputs, TWrite<CExprTypeId>> access, AST::Id nodeId)
	{
		const AST::Id outputId = nodeId;    // Output in unary operator is same as the node itself
		const auto& inputs     = access.Get<const CExprInputs>(nodeId);
		const OutputId* linkedOutputId = inputs.linkedOutputs.At(0);
		if (linkedOutputId && linkedOutputId->pinId != AST::NoId)
		{
			return Types::CopyExpressionType(access, linkedOutputId->pinId, outputId);
		}
		return false;
	}

	bool PropagateBinaryOperator(TAccess<CExprInputs, TWrite<CExprTypeId>> access, AST::Id nodeId)
	{
		const auto& inputs = access.Get<const CExprInputs>(nodeId);
		AST::Id outputId   = nodeId;    // Output in binary operator is same as the node itself
		if (inputs.pinIds.Size() == 2) [[likely]]
		{
			const OutputId firstLinkedOutputId = inputs.linkedOutputs[0];
			// Set input types
			Types::CopyExpressionType(access, firstLinkedOutputId.pinId, inputs.pinIds[0]);
			Types::CopyExpressionType(access, inputs.linkedOutputs[1].pinId, inputs.pinIds[1]);
			// Set output type from first connection
			return Types::CopyExpressionType(access, firstLinkedOutputId.pinId, outputId);
		}
		return false;
	}

	void PropagateExpressionTypes(PropagateExpressionTypesAccess access)
	{
		TArray<AST::Id> dirtyTypeIds = ecs::ListAll<CType, CChanged>(access);

		TArray<AST::Id> dirtyNodeIds;
		AST::Hierarchy::GetChildren(access, dirtyTypeIds, dirtyNodeIds);

		// Make sure the nodes have inputs and outputs
		ecs::ExcludeIfNot<CExprInputs, CExprOutputs>(access, dirtyNodeIds);

		// Only Unary and Binary operators propagate as of right now
		ecs::ExcludeIf(dirtyNodeIds, [&access](AST::Id id) {
			return !access.Has<CExprUnaryOperator>(id) && !access.Has<CExprBinaryOperator>(id);
		});

		bool anyPropagated;
		while (!dirtyNodeIds.IsEmpty())    // Repeat until nothing to propagate
		{
			bool anyPropagated = false;
			// Propagate all dirty nodes, remove successfully propagated ones
			for (i32 i = dirtyNodeIds.Size() - 1; i >= 0; --i)
			{
				const AST::Id nodeId = dirtyNodeIds[i];

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
		for (AST::Id id : callExprs)
		{
			auto& expr           = access.Get<const CExprType>(id);
			const AST::Id typeId = AST::FindIdFromNamespace(access, expr.type);
			if (!IsNone(typeId))
			{
				access.Add(id, CExprTypeId{.id = typeId, .mode = expr.mode});
			}
		}
	}
}    // namespace rift::TypeSystem
