// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Tree.h"
#include "AST/Uniques/CEntityLoadUnique.h"

#include <Containers/ArrayView.h>


namespace Rift::Loading
{
	inline bool MarkPendingLoad(const AST::Tree& ast, TArrayView<AST::Id> nodes)
	{
		if (nodes.IsEmpty())
		{
			return false;
		}

		auto& loadData = ast.GetUnique<CEntityLoadUnique>();
		auto filesView = ast.MakeView<const CFileRef>();


		return true;
	}
}    // namespace Rift::Loading
