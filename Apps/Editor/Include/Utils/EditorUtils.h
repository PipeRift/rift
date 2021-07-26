// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"


namespace Rift::Editor
{
	void OpenType(AST::Tree& ast, AST::Id typeId);
	void CloseType(AST::Tree& ast, AST::Id typeId);
}    // namespace Rift::Editor
