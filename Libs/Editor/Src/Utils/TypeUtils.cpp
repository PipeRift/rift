// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/TypeUtils.h"

#include "AST/Serialization.h"

#include <Pipe/Core/Checks.h>


namespace rift::Types
{
	void OpenEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id)
	{
		Check(access.Has<CType>(id));
		if (auto* editor = access.TryGet<CTypeEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CTypeEditor>(id);
		}
	}

	void CloseEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id)
	{
		Check(access.Has<CType>(id));
		access.Remove<CTypeEditor>(id);
	}

	bool IsEditorOpen(TAccessRef<CTypeEditor> access, AST::Id id)
	{
		return access.Has<CTypeEditor>(id);
	}
}    // namespace rift::Types
