// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CIdentifier.h"
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
		auto onlyTypesView = ast.Filter<const CType>();
		typeList.typesByName.Reserve(u32(onlyTypesView.Size()));

		auto typesView = ast.Filter<const CType, const CIdentifier>();
		for (AST::Id typeId : typesView)
		{
			const CIdentifier& identifier = typesView.Get<const CIdentifier>(typeId);
			typeList.typesByName.Insert(identifier.name, typeId);
		}

		ast.OnAdd<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CIdentifier* identifier = ast.TryGet<const CIdentifier>(id))
				{
					typeList.typesByName.Insert(identifier->name, id);
				}
			}
		});

		ast.OnRemove<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CIdentifier* identifier = ast.TryGet<const CIdentifier>(id))
				{
					typeList.typesByName.Remove(identifier->name);
				}
			}
		});
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
