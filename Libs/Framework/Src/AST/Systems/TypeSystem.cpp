// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloating.h"
#include "AST/Components/CLiteralIntegral.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CType.h"
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
				access.Add<CExprType>(id, {typeId});
			}
		}
	}

	void PropagateExpressionTypes(AST::Tree& ast)
	{
		TArray<AST::Id> literals =
		    ecs::ListAny<CLiteralBool, CLiteralIntegral, CLiteralFloating, CLiteralString>(ast);
	}
}    // namespace rift::TypeSystem
