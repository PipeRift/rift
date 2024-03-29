// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclNative.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclType.h>
#include <AST/Components/CNamespace.h>
#include <AST/Tree.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/PipeECS.h>


namespace rift::Editor
{
	bool TypeCombo(AST::TAccessRef<AST::CNamespace, AST::CDeclType, AST::CDeclNative,
	                   AST::CDeclStruct, AST::CDeclClass>
	                   access,
	    p::StringView label, AST::Id& selectedTypeId);

	bool InputLiteralValue(AST::Tree& ast, p::StringView label, AST::Id literalId);
}    // namespace rift::Editor
