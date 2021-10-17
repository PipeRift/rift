// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/TypeSystem.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CType.h"
#include "AST/Tree.h"
#include "AST/Uniques/CTypeListUnique.h"


namespace Rift::TypeSystem
{
	void OnNewType(AST::Tree::Registry& registry, AST::Id typeId)
	{
		if (const CIdentifier* identifier = registry.try_get<const CIdentifier>(typeId))
		{
			auto& typeList = registry.ctx<CTypeListUnique>();
			typeList.types.Insert(identifier->name, typeId);
		}
	}

	void OnDeleteType(AST::Tree::Registry& registry, AST::Id typeId)
	{
		if (const CIdentifier* identifier = registry.try_get<const CIdentifier>(typeId))
		{
			auto& typeList = registry.ctx<CTypeListUnique>();
			typeList.types.Remove(identifier->name);
		}
	}

	void Init(AST::Tree& ast)
	{
		auto& typeList = ast.GetOrSetUnique<CTypeListUnique>();
		typeList.types.Empty();

		// Cache existing types
		auto onlyTypesView = ast.MakeView<const CType>();
		typeList.types.Reserve(onlyTypesView.Size());

		auto typesView = ast.MakeView<const CType, const CIdentifier>();
		for (AST::Id typeId : typesView)
		{
			const CIdentifier& identifier = typesView.Get<const CIdentifier>(typeId);
			typeList.types.Insert(identifier.name, typeId);
		}

		ast.OnConstruct<CType>().connect<&OnNewType>();
		ast.OnDestroy<CType>().connect<&OnDeleteType>();
	}

	void RunChecks(AST::Tree& ast)
	{
		// Load pending files
	}
}    // namespace Rift::TypeSystem
