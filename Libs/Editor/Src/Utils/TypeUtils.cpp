// Copyright 2015-2023 Piperift - All rights reserved

#include "Utils/TypeUtils.h"

#include <Pipe/Core/Checks.h>
#include <Pipe/PipeECS.h>


namespace rift::Editor
{
	void OpenType(TAccessRef<TWrite<CTypeEditor>, AST::CDeclType> access, AST::Id id)
	{
		Check(access.Has<AST::CDeclType>(id));
		if (auto* editor = access.TryGet<CTypeEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CTypeEditor>(id);
		}
	}

	void CloseType(TAccessRef<TWrite<CTypeEditor>, AST::CDeclType> access, AST::Id id)
	{
		Check(access.Has<AST::CDeclType>(id));
		access.Remove<CTypeEditor>(id);
	}

	bool IsTypeOpen(TAccessRef<CTypeEditor> access, AST::Id id)
	{
		return access.Has<CTypeEditor>(id);
	}
}    // namespace rift::Editor
