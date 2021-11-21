// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CType.h"
#include "AST/Statics/STypeList.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Tree.h"


namespace Rift::TypeSystem
{
	void Init(AST::Tree& ast)
	{
		auto& typeList = ast.GetOrSetStatic<CTypeListUnique>();
		typeList.types.Empty();

		// Cache existing types
		auto onlyTypesView = ast.Query<const CType>();
		typeList.types.Reserve(u32(onlyTypesView.Size()));

		auto typesView = ast.Query<const CType, const CIdentifier>();
		for (AST::Id typeId : typesView)
		{
			const CIdentifier& identifier = typesView.Get<const CIdentifier>(typeId);
			typeList.types.Insert(identifier.name, typeId);
		}

		ast.OnAdd<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CIdentifier* identifier = ast.TryGet<const CIdentifier>(id))
				{
					typeList.types.Insert(identifier->name, id);
				}
			}
		});

		ast.OnRemove<CType>().Bind([&ast, &typeList](auto ids) {
			for (AST::Id id : ids)
			{
				if (const CIdentifier* identifier = ast.TryGet<const CIdentifier>(id))
				{
					typeList.types.Remove(identifier->name);
				}
			}
		});
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
