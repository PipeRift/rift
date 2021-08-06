// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "AST/Uniques/CLoadQueueUnique.h"

#include <Containers/ArrayView.h>


namespace Rift::Loading
{
	inline bool MarkPendingLoad(
	    AST::Tree& ast, TArrayView<AST::Id> nodes, LoadMode mode = LoadMode::Async)
	{
		if (nodes.IsEmpty())
		{
			return false;
		}

		auto& loadQueue = ast.GetUnique<CLoadQueueUnique>();
		if (mode == LoadMode::Sync)
		{
			loadQueue.pendingSyncLoad.Append(nodes);
		}
		else
		{
			loadQueue.pendingAsyncLoad.Append(nodes);
		}
		return true;
	}
}    // namespace Rift::Loading
