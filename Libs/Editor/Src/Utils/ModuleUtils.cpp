// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/ModuleUtils.h"

#include <Pipe/Core/Checks.h>


namespace rift::editor
{
	void OpenModuleEditor(p::TIdScopeRef<Writes<CModuleEditor>, ast::CModule> scope, ast::Id id)
	{
		P_Check(scope.Has<ast::CModule>(id));
		if (auto* editor = scope.TryGet<CModuleEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			scope.Add<CModuleEditor>(id);
		}
	}

	void CloseModuleEditor(p::TIdScopeRef<Writes<CModuleEditor>, ast::CModule> scope, ast::Id id)
	{
		P_Check(scope.Has<ast::CModule>(id));
		scope.Remove<CModuleEditor>(id);
	}

	bool IsEditingModule(p::TIdScopeRef<CModuleEditor> scope, ast::Id id)
	{
		return scope.Has<CModuleEditor>(id);
	}
}    // namespace rift::editor
