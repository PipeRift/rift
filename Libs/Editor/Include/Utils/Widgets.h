// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclNative.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/CType.h>
#include <AST/Tree.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/ECS/Filtering.h>


namespace rift::Editor
{
	bool TypeCombo(TAccessRef<CNamespace, CType, CDeclNative, CDeclStruct, CDeclClass> access,
	    StringView label, AST::Id& selectedTypeId);

	bool InputLiteralValue(AST::Tree& ast, StringView label, AST::Id literalId);
}    // namespace rift::Editor
