// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Filtering.h"


namespace Rift::AST
{
	void RemoveIf(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIfSwap(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void RemoveIfStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void RemoveIfNot(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIfSwap(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}

	void RemoveIfNotStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}
}    // namespace Rift::AST
