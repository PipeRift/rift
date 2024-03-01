// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/TypeUtils.h"

#include <Pipe/Core/Checks.h>
#include <PipeECS.h>


namespace rift::editor
{
	void OpenType(p::TAccessRef<TWrite<CTypeEditor>, ast::CDeclType> access, ast::Id id)
	{
		Check(access.Has<ast::CDeclType>(id));
		if (auto* editor = access.TryGet<CTypeEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CTypeEditor>(id);
		}
	}

	void CloseType(p::TAccessRef<TWrite<CTypeEditor>, ast::CDeclType> access, ast::Id id)
	{
		Check(access.Has<ast::CDeclType>(id));
		access.Remove<CTypeEditor>(id);
	}

	bool IsTypeOpen(p::TAccessRef<CTypeEditor> access, ast::Id id)
	{
		return access.Has<CTypeEditor>(id);
	}
}    // namespace rift::editor
