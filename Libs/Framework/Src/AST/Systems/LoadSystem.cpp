// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Tree.h"
#include "AST/Uniques/CEntityLoadUnique.h"


namespace Rift::LoadSystem
{
	void Init(AST::Tree& ast)
	{
		ast.SetUnique<CEntityLoadUnique>();
	}

	void Run(AST::Tree& ast)
	{
		auto& loadData = ast.GetUnique<CEntityLoadUnique>();
	}
}    // namespace Rift::LoadSystem
