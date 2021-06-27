// Copyright 2015-2020 Piperift - All rights reserved
#include "AST/Systems/AssetSystem.h"
#include "AST/Tree.h"
#include "AST/Uniques/CAssetsUnique.h"


namespace Rift::AssetSystem
{
	void Init(AST::Tree& ast)
	{
		ast.SetUnique<CAssetsUnique>();
	}

	void Run(AST::Tree& ast)
	{
		auto& assets = ast.GetUnique<CAssetsUnique>();
	}
}    // namespace Rift
