// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/ModuleUtils.h"

#include <Pipe/Core/Checks.h>


namespace rift::Editor
{
	void OpenModule(TAccessRef<TWrite<CModuleEditor>, AST::CModule> access, AST::Id id)
	{
		Check(access.Has<AST::CModule>(id));
		if (auto* editor = access.TryGet<CModuleEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CModuleEditor>(id);
		}
	}

	void CloseModule(TAccessRef<TWrite<CModuleEditor>, AST::CModule> access, AST::Id id)
	{
		Check(access.Has<AST::CModule>(id));
		access.Remove<CModuleEditor>(id);
	}

	bool IsModuleOpen(TAccessRef<CModuleEditor> access, AST::Id id)
	{
		return access.Has<CModuleEditor>(id);
	}
}    // namespace rift::Editor
