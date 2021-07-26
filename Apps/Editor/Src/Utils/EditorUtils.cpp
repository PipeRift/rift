// Copyright 2015-2020 Piperift - All rights reserved

#include "Components/CTypeEditor.h"
#include "Utils/EditorUtils.h"

#include <AST/Components/CType.h>
#include <Misc/Checks.h>


namespace Rift::Modules
{
	void OpenType(AST::Tree& ast, AST::Id typeId)
	{
		Check(ast.Has<CType>(typeId));

		ast.Add<CTypeEditor>(typeId);
	}

	void CloseType(AST::Tree& ast, AST::Id typeId)
	{
		Check(ast.Has<CType>(typeId));

		ast.Remove<CTypeEditor>(typeId);
	}
}    // namespace Rift::Modules
