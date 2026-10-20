// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/TypeUtils.h"

#include "Systems/EditorSystem.h"

#include <IconsFontAwesome5.h>
#include <Pipe/Core/Checks.h>
#include <PipeECS.h>


namespace rift::editor
{
	StringView GetTypeIcon(ast::Tree& ast, ast::Id typeId)
	{
		if (ast::IsStructType(ast, typeId))
		{
			return ICON_FA_FILE_ALT;
		}
		if (ast::IsClassType(ast, typeId))
		{
			return ICON_FA_FILE_INVOICE;
		}
		if (ast::IsStaticType(ast, typeId))
		{
			return ICON_FA_FILE_WORD;
		}
		return "";
	}

	void OpenType(p::TIdScopeRef<Writes<CTypeEditor>, ast::CDeclType> scope, ast::Id id)
	{
		P_Check(scope.Has<ast::CDeclType>(id));
		if (auto* editor = scope.TryGet<CTypeEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			scope.Add<CTypeEditor>(id);
			EditorSystem::OnTypeEditorOpen(static_cast<ast::Tree&>(scope.GetContext()), id);
		}
	}

	void CloseType(p::TIdScopeRef<Writes<CTypeEditor>, ast::CDeclType> scope, ast::Id id)
	{
		P_Check(scope.Has<ast::CDeclType>(id));
		scope.Remove<CTypeEditor>(id);
	}

	bool IsTypeOpen(p::TIdScopeRef<CTypeEditor> scope, ast::Id id)
	{
		return scope.Has<CTypeEditor>(id);
	}
}    // namespace rift::editor
