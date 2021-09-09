// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CType.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Tree.h"
#include "AST/Uniques/CTypeListUnique.h"


namespace Rift::TypeSystem
{
	void OnNewType(AST::Tree::Registry& registry, AST::Id typeId)
	{
		const CType& type = registry.get<const CType>(typeId);
		auto& typeList = registry.ctx<CTypeListUnique>();
		typeList.types.Insert(type.id, typeId);
	}

	void OnDeleteType(AST::Tree::Registry& registry, AST::Id typeId)
	{
		CType& type    = registry.get<CType>(typeId);
		auto& typeList = registry.ctx<CTypeListUnique>();
		typeList.types.Remove(type.id);
	}

	void Init(AST::Tree& ast)
	{
		auto& typeList = ast.GetOrSetUnique<CTypeListUnique>();
		typeList.types.Empty();

		// Cache existing types
		auto typesView = ast.MakeView<const CType>();
		typeList.types.Reserve(typesView.Size());
		for (AST::Id typeId : typesView)
		{
			const CType& type = typesView.Get<const CType>(typeId);
			typeList.types.Insert(type.id, typeId);
		}

		ast.OnConstruct<CType>().connect<&OnNewType>();
		ast.OnDestroy<CType>().connect<&OnDeleteType>();
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
