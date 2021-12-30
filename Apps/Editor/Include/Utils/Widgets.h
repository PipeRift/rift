// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <AST/Tree.h>
#include <Strings/StringView.h>


namespace Rift::Editor
{
	bool TypeCombo(AST::Tree& ast, StringView label, AST::Id& selectedTypeId);

	bool InputLiteralValue(AST::Tree& ast, StringView label, AST::Id literalId);
}    // namespace Rift::Editor
