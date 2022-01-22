// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/TypeUtils.h"

#include "AST/Serialization.h"
#include "Components/CTypeEditor.h"

#include <AST/Components/CType.h>
#include <Misc/Checks.h>


namespace Rift::Types
{
	void OpenType(AST::Tree& ast, AST::Id typeId)
	{
		Check(ast.Has<CType>(typeId));

		if (auto* editor = ast.TryGet<CTypeEditor>(typeId))
		{
			editor->pendingFocus = true;
		}
		else
		{
			ast.Add<CTypeEditor>(typeId);
		}
	}

	void CloseType(AST::Tree& ast, AST::Id typeId)
	{
		Check(ast.Has<CType>(typeId));

		ast.Remove<CTypeEditor>(typeId);
	}

	bool IsTypeOpen(AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CTypeEditor>(typeId);
	}

	void Changed(AST::Id typeId, StringView name) {}
}    // namespace Rift::Types
