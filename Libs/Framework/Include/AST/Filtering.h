// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Access.h"
#include "AST/Tree.h"


namespace Rift::AST
{
	void RemoveIf(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfNot(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);
	void RemoveIfNotStable(const Pool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/**
	 * Removes ids if they have a component. Does not guarantee order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see RemoveIfStable(), RemoveIfNot()
	 */
	template<typename T, typename AccessType>
	void RemoveIf(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIf(access.template GetPool<const T>(), ids, shouldShrink);
	}

	/**
	 * Removes ids if they have a component. Guarantees order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see RemoveIf(), RemoveIfNotStable()
	 */
	template<typename T, typename AccessType>
	void RemoveIfStable(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfStable(access.template GetPool<const T>(), ids, shouldShrink);
	}

	/**
	 * Removes ids if they don't have a component. Does not guarantee order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see RemoveIfNotStable(), RemoveIf()
	 */
	template<typename T, typename AccessType>
	void RemoveIfNot(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfNot(access.template GetPool<const T>(), ids, shouldShrink);
	}

	/**
	 * Removes ids if they don't have a component. Guarantees order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see RemoveIfNot(), RemoveIfStable()
	 */
	template<typename T, typename AccessType>
	void RemoveIfNotStable(
	    const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		RemoveIfNotStable(access.template GetPool<const T>(), ids, shouldShrink);
	}

	/**
	 * Finds all ids containing all of the specified components
	 *
	 * @param access from where to access pools
	 * @param ids array where matching ids will be added
	 * @see ListAny()
	 */
	template<typename AccessType, typename... T>
	void ListAll(const AccessType& access, TArray<Id>& ids)
	{
		ZoneScoped;
		TArray<const Pool*> pools{access.template GetPool<const T>()...};
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

		ids.Empty();
		ids.Append(iterablePool->begin(), iterablePool->end());

		for (const Pool* pool : pools)
		{
			RemoveIfNot(pool, ids, false);
		}
	}

	/**
	 * Finds all ids containing all of the specified components
	 *
	 * @param access from where to access pools
	 * @return ids array with matching ids
	 * @see ListAny()
	 */
	template<typename... T, typename AccessType>
	TArray<Id> ListAll(const AccessType& access)
	{
		TArray<Id> ids;
		ListAll<AccessType, T...>(access, ids);
		return Move(ids);
	}

	/**
	 * Finds all ids containing any of the specified components
	 *
	 * @param access from where to access pools
	 * @param ids array where matching ids will be added
	 * @see ListAll()
	 */
	template<typename AccessType, typename... T>
	void ListAny(const AccessType& access, TArray<Id>& ids)
	{
		ZoneScoped;
		TArray<const Pool*> pools{access.template GetPool<const T>()...};

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

	/**
	 * Finds all ids containing any of the specified components
	 *
	 * @param access from where to access pools
	 * @return ids array with matching ids
	 * @see ListAll()
	 */
	template<typename... T, typename AccessType>
	TArray<Id> ListAny(const AccessType& access)
	{
		TArray<Id> ids;
		ListAny<AccessType, T...>(access, ids);
		return Move(ids);
	}
}    // namespace Rift::AST
