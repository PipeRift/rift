// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Tree.h>
#include <Pipe/Core/EnumFlags.h>
#include <Pipe/Core/StringView.h>
#include <PipeECS.h>


namespace rift::editor
{
	enum class TypeComboFlags
	{
		None           = 0,
		SelectedAsChip = 1 << 0,    // Draws the selected type as a chip
		ShowCategories = 1 << 1,    // Groups types by category
	};
	P_DEFINE_FLAG_OPERATORS(TypeComboFlags)

	bool TypeCombo(p::TIdScopeRef<ast::CNamespace, ast::CDeclType, ast::CDeclNative,
	                   ast::CDeclStruct, ast::CDeclClass>
	                   scope,
	    p::StringView label, ast::Id& selectedTypeId,
	    TypeComboFlags flags = TypeComboFlags::SelectedAsChip);

	bool InputLiteralValue(ast::Tree& ast, p::StringView label, ast::Id literalId);
}    // namespace rift::editor
