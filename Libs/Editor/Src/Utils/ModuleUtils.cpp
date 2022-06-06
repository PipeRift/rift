// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/ModuleUtils.h"

#include "AST/Serialization.h"

#include <Core/Checks.h>


namespace rift::Modules
{
	void OpenEditor(TAccessRef<TWrite<CModuleEditor>, CModule> access, AST::Id id)
	{
		Check(access.Has<CModule>(id));
		if (auto* editor = access.TryGet<CModuleEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CModuleEditor>(id);
		}
	}

	void CloseEditor(TAccessRef<TWrite<CModuleEditor>, CModule> access, AST::Id id)
	{
		Check(access.Has<CModule>(id));
		access.Remove<CModuleEditor>(id);
	}

	bool IsEditorOpen(TAccessRef<CModuleEditor> access, AST::Id id)
	{
		return access.Has<CModuleEditor>(id);
	}
}    // namespace rift::Modules
