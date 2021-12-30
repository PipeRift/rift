// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CType.h"
#include "AST/Statics/STypes.h"
#include "AST/Tree.h"


namespace Rift::TypeSystem
{
	void Init(AST::Tree& ast)
	{
		auto& typeList = ast.GetOrSetStatic<STypes>();
		typeList.typesByName.Empty();

		// Cache existing types
		auto typesView = ast.Filter<const CType>();
		typeList.typesByName.Reserve(u32(typesView.Size()));
		for (AST::Id typeId : typesView)
		{
			const CType& type = typesView.Get<const CType>(typeId);
			typeList.typesByName.Insert(type.name, typeId);
		}

		ast.OnAdd<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CType* type = ast.TryGet<const CType>(id))
				{
					typeList.typesByName.Insert(type->name, id);
				}
			}
		});

		ast.OnRemove<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CType* type = ast.TryGet<const CType>(id))
				{
					typeList.typesByName.Remove(type->name);
				}
			}
		});
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
