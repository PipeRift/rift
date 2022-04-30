// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Filtering.h"


namespace Rift::AST
{
	void RemoveIf(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (pool->Has(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	void RemoveIfStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		ids.RemoveIf(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void RemoveIfNot(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (!pool->Has(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	void RemoveIfNotStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		ids.RemoveIf(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}

	void GetIf(const Pool* pool, const TArray<Id>& source, TArray<Id>& results)
	{
		ZoneScoped;
		if (pool)
		{
			results.ReserveMore(Math::Min(i32(pool->Size()), source.Size()));
			for (AST::Id id : source)
			{
				if (pool->Has(id))
				{
					results.Add(id);
				}
			}
		}
	}

	void GetIfNot(const Pool* pool, const TArray<Id>& source, TArray<Id>& results)
	{
		ZoneScoped;
		if (pool)
		{
			results.ReserveMore(source.Size());
			for (AST::Id id : source)
			{
				if (!pool->Has(id))
				{
					results.Add(id);
				}
			}
		}
		else
		{
			// No pool means no id has the component
			results = source;
		}
	}

	void ExtractIf(
	    const Pool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(Math::Min(i32(pool->Size()), source.Size()));
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const AST::Id id = source[i];
			if (pool->Has(id))
			{
				source.RemoveAtSwapUnsafe(i);
				results.Add(id);
			}
		}
		if (shouldShrink)
		{
			source.Shrink();
		}
	}

	void ExtractIfStable(
	    const Pool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(Math::Min(i32(pool->Size()), source.Size()));
		source.RemoveIf(
		    [pool, &results](Id id) {
			if (pool->Has(id))
			{
				results.Add(id);
				return true;
			}
			return false;
		    },
		    shouldShrink);
	}

	void ExtractIfNot(
	    const Pool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(source.Size());
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const AST::Id id = source[i];
			if (!pool->Has(id))
			{
				source.RemoveAtSwapUnsafe(i);
				results.Add(id);
			}
		}
		if (shouldShrink)
		{
			source.Shrink();
		}
	}

	void ExtractIfNotStable(
	    const Pool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(Math::Min(i32(pool->Size()), source.Size()));
		source.RemoveIf(
		    [pool, &results](Id id) {
			if (!pool->Has(id))
			{
				results.Add(id);
				return true;
			}
			return false;
		    },
		    shouldShrink);
	}

	void ListAll(TArray<const Pool*> pools, TArray<Id>& ids)
	{
		ZoneScoped;
		for (const Pool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}
		}

		const i32 smallestIdx    = GetSmallestPool(pools);
		const Pool* iterablePool = pools[smallestIdx];
		pools.RemoveAtSwap(smallestIdx);

		ids.Empty(false);
		ids.Append(iterablePool->begin(), iterablePool->end());

		for (const Pool* pool : pools)
		{
			RemoveIfNot(pool, ids, false);
		}
	}

	void ListAny(const TArray<const Pool*>& pools, TArray<Id>& ids)
	{
		ZoneScoped;
		for (const Pool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}
		}

		ids.Empty();
		for (const Pool* pool : pools)
		{
			ids.Append(pool->begin(), pool->end());
		}
	}

	void ListAnyUnique(const TArray<const Pool*>& pools, TArray<Id>& ids)
	{
		ZoneScoped;
		i32 maxPossibleSize = 0;
		for (const Pool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}

			maxPossibleSize += pool->Size();
		}

		TSet<Id> idsSet;
		idsSet.Reserve(maxPossibleSize);
		for (const Pool* pool : pools)
		{
			for (Id id : *pool)
			{
				idsSet.Insert(id);
			}
		}

		ids.Empty();
		ids.Append(idsSet.begin(), idsSet.end());
	}
}    // namespace Rift::AST
