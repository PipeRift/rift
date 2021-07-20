// Copyright 2015-2020 Piperift - All rights reserved
#pragma once


namespace Rift::AST
{
	struct Tree;
}

namespace Rift::ModuleSystem
{
	void ScanModuleTypes(AST::Tree& ast);
}    // namespace Rift::ModuleSystem
