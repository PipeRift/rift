// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Pool.h"


namespace Rift::AST
{
	i32 GetSmallestPool(TSpan<const Pool*> pools)
	{
		sizet minSize = std::numeric_limits<sizet>::max();
		i32 minIndex  = NO_INDEX;
		for (i32 i = 0; i < pools.Size(); ++i)
		{
			const i32 size = pools[i]->Size();
			if (size < minSize)
			{
				minSize  = size;
				minIndex = i;
			}
		}
		return minIndex;
	}
}    // namespace Rift::AST
