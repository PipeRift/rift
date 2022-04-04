// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CType.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"


namespace Rift::TypeSystem
{
	void Init(AST::Tree& ast)
	{
		auto& types = ast.GetOrSetStatic<STypes>();
		types.typesByName.Empty();

		// Cache existing types
		auto typesView = ast.Filter<const CType>();
		types.typesByName.Reserve(u32(typesView.Size()));
		for (AST::Id typeId : typesView)
		{
			const CType& type = typesView.Get<const CType>(typeId);
			types.typesByName.Insert(type.name, typeId);
		}

		ast.OnAdd<CType>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (const CType* type = ast.template TryGet<const CType>(id))
				{
					types.typesByName.Insert(type->name, id);
				}
			}
		});

		ast.OnRemove<CType>().Bind([](auto& ast, auto ids) {
			auto& types = ast.template GetOrSetStatic<STypes>();
			for (AST::Id id : ids)
			{
				if (const CType* type = ast.template TryGet<const CType>(id))
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
					const CFileRef& file = ast.template Get<const CFileRef>(id);
					const Name pathName{Paths::ToString(file.path)};
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
					const CFileRef& file = ast.template Get<const CFileRef>(id);
					const Name pathName{Paths::ToString(file.path)};
					types.typesByPath.Remove(pathName);
				}
			}
		});
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
