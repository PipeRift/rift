// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Access.h"
#include "AST/Tree.h"


namespace Rift::AST
{
	template<typename AccessType, typename... T>
	void ListAll(const AccessType& access, TArray<Id>& ids)
	{
		TArray<const Pool*> pools{access.GetPool<const T>()...};

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
			RemoveIf(pool, ids, false);
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

	void RemoveIf(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	void RemoveIfStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	template<typename T, typename AccessType>
	void RemoveIf(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIf(access.GetPool<const T>(), ids, shouldShrink);
	}

	template<typename T, typename AccessType>
	void RemoveIfStable(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfStable(access.GetPool<const T>(), ids, shouldShrink);
	}
}    // namespace Rift::AST
