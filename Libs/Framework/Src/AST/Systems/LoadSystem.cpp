// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Tree.h"
#include "AST/Uniques/CLoadQueueUnique.h"
#include "AST/Uniques/CStringLoadUnique.h"


namespace Rift::LoadSystem
{
	void Init(AST::Tree& ast)
	{
		ast.SetUnique<CLoadQueueUnique>();
		ast.SetUnique<CStringLoadUnique>();
	}

	void Run(AST::Tree& ast)
	{
		auto& loadQueue  = ast.GetUnique<CLoadQueueUnique>();
		auto& stringLoad = ast.GetUnique<CStringLoadUnique>();
	}
}    // namespace Rift::LoadSystem
