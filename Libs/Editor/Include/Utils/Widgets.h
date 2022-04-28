// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclNative.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CType.h>
#include <AST/Filtering.h>
#include <AST/Tree.h>
#include <Strings/StringView.h>


namespace Rift::Editor
{
	bool TypeCombo(TAccessRef<CType, CDeclNative, CDeclStruct, CDeclClass> access, StringView label,
	    AST::Id& selectedTypeId);

	bool InputLiteralValue(AST::Tree& ast, StringView label, AST::Id literalId);
}    // namespace Rift::Editor
