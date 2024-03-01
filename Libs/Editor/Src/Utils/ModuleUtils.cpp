// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/ModuleUtils.h"

#include <Pipe/Core/Checks.h>


namespace rift::editor
{
	void OpenModuleEditor(p::TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id)
	{
		Check(access.Has<ast::CModule>(id));
		if (auto* editor = access.TryGet<CModuleEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CModuleEditor>(id);
		}
	}

	void CloseModuleEditor(p::TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id)
	{
		Check(access.Has<ast::CModule>(id));
		access.Remove<CModuleEditor>(id);
	}

	bool IsEditingModule(p::TAccessRef<CModuleEditor> access, ast::Id id)
	{
		return access.Has<CModuleEditor>(id);
	}
}    // namespace rift::editor
