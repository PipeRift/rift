// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/TypeUtils.h"

#include "AST/Serialization.h"

#include <Pipe/Core/Checks.h>


namespace rift::Editor
{
	void OpenType(TAccessRef<TWrite<CTypeEditor>, AST::CType> access, AST::Id id)
	{
		Check(access.Has<AST::CType>(id));
		if (auto* editor = access.TryGet<CTypeEditor>(id))
		{
			editor->pendingFocus = true;
		}
		else
		{
			access.Add<CTypeEditor>(id);
		}
	}

	void CloseType(TAccessRef<TWrite<CTypeEditor>, AST::CType> access, AST::Id id)
	{
		Check(access.Has<AST::CType>(id));
		access.Remove<CTypeEditor>(id);
	}

	bool IsTypeOpen(TAccessRef<CTypeEditor> access, AST::Id id)
	{
		return access.Has<CTypeEditor>(id);
	}
}    // namespace rift::Editor
