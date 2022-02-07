// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Access.h"
#include "AST/Tree.h"


namespace Rift::AST
{
	template<typename AccessType, typename... T>
	void ListAll(const AccessType& access, TArray<Id>& ids)
	{
		TArray<const Pool*> pools{access.template GetPool<const T>()...};
		for (const Pool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Does the provided access have all components to "
			        "filter?"))
			{
				return;
			}
		}

		const i32 smallestIdx    = GetSmallestPool(pools);
		const Pool* iterablePool = pools[smallestIdx];
		pools.RemoveAtSwap(smallestIdx);

		ids.Reserve(ids.Size() + iterablePool->Size());
		for (Id id : *iterablePool)
		{
			ids.Add(id);
		}

		for (const Pool* pool : pools)
		{
			RemoveIfNot(pool, ids, false);
		}
		ids.Shrink();
	}

	template<typename... T, typename AccessType>
	TArray<Id> ListAll(const AccessType& access)
	{
		TArray<Id> ids;
		ListAll<AccessType, T...>(access, ids);
		return ids;
	}

	template<typename AccessType, typename... T>
	void ListAny(const AccessType& access, TArray<Id>& ids)
	{
		TArray<const Pool*> pools{access.template GetPool<const T>()...};

		i32 maxPossibleSize = 0;
		for (const Pool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Does the provided access have all components to "
			        "filter?"))
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
		ids.Append(idsSet.begin(), idsSet.end());
	}

	template<typename... T, typename AccessType>
	TArray<Id> ListAny(const AccessType& access)
	{
		TArray<Id> ids;
		ListAny<AccessType, T...>(access, ids);
		return ids;
	}

	void RemoveIf(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfNot(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfNotStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	template<typename T, typename AccessType>
	void RemoveIf(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIf(access.template GetPool<const T>(), ids, shouldShrink);
	}

	template<typename T, typename AccessType>
	void RemoveIfStable(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfStable(access.template GetPool<const T>(), ids, shouldShrink);
	}


	template<typename T, typename AccessType>
	void RemoveIfNot(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfNot(access.template GetPool<const T>(), ids, shouldShrink);
	}

	template<typename T, typename AccessType>
	void RemoveIfNotStable(
	    const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfNotStable(access.template GetPool<const T>(), ids, shouldShrink);
	}
}    // namespace Rift::AST
