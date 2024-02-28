// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Tree.h>
#include <Pipe/Core/StringView.h>
#include <PipeECS.h>


namespace rift::editor
{
	bool TypeCombo(p::TAccessRef<ast::CNamespace, ast::CDeclType, ast::CDeclNative,
	                   ast::CDeclStruct, ast::CDeclClass>
	                   access,
	    p::StringView label, ast::Id& selectedTypeId);

	bool InputLiteralValue(ast::Tree& ast, p::StringView label, ast::Id literalId);
}    // namespace rift::editor
