// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/ModuleUtils.h"

#include <Pipe/Core/Checks.h>


namespace rift::Editor
{
	void OpenModuleEditor(TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id)
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

	void CloseModuleEditor(TAccessRef<TWrite<CModuleEditor>, ast::CModule> access, ast::Id id)
	{
		Check(access.Has<ast::CModule>(id));
		access.Remove<CModuleEditor>(id);
	}

	bool IsEditingModule(TAccessRef<CModuleEditor> access, ast::Id id)
	{
		return access.Has<CModuleEditor>(id);
	}
}    // namespace rift::Editor
