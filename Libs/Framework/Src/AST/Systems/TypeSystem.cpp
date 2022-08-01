// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloating.h"
#include "AST/Components/CLiteralIntegral.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Id.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"

#include <Pipe/ECS/Filtering.h>


namespace rift::TypeSystem
{
	void Init(AST::Tree& ast)
	{
		TAccess<CType, CNamespace> access{ast};

		auto& types = ast.GetOrSetStatic<STypes>();
		types.typesByName.Clear();

		// Cache existing types

		auto typeIds = ecs::ListAll<CType, CNamespace>(access);
		types.typesByName.Reserve(u32(typeIds.Size()));
		for (AST::Id typeId : typeIds)
		{
			const auto& type = access.Get<const CNamespace>(typeId);
			types.typesByName.Insert(type.name, typeId);
		}

		ast.OnAdd<CType>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (const auto* type = ast.template TryGet<const CNamespace>(id))
				{
					types.typesByName.Insert(type->name, id);
				}
			}
		});

		ast.OnRemove<CType>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (const auto* type = ast.template TryGet<const CNamespace>(id))
				{
					types.typesByName.Remove(type->name);
				}
			}
		});

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
				access.Add<CExprTypeId>(id, {typeId});
			}
		}
	}

	void PropagateExpressionTypes(AST::Tree& ast)
	{
		// TODO: Only do this with dirty types

		// TODO: In editor, also get binary/unary operators not connected to anything
		TArray<AST::Id> propagationSources;
		ecs::GetIf<CExprCall>(ast, propagationSources);

		TArray<OutputId> pendingToPropagate;

		// Find connected to sources
		for (AST::Id sourceId : propagationSources)
		{
			if (auto* inputs = ast.TryGet<CExprInputs>(sourceId)) [[likely]]
			{
				pendingToPropagate.Append(inputs->linkedOutputs);
			}
		}

		while (!pendingToPropagate.IsEmpty())
		{
			for (i32 i = pendingToPropagate.Size() - 1; i >= 0; --i)
			{
				OutputId id = pendingToPropagate[i];
			}
			// For each pending propagate (iterate backwards)
			// Get connected
			// Those connected that have invalid type, add to pending propagate
			// If all connected are propagated, propagate this and remove from pending propagate
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
				access.Add(id, CExprTypeId{typeId});
			}
		}
	}
}    // namespace rift::TypeSystem
