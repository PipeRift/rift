// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Pool.h"
#include "AST/Types.h"


namespace Rift::AST
{
	template<typename Access>
	struct TFilter
	{
		using Iterator        = typename Access::Iterator;
		using ReverseIterator = typename Access::ReverseIterator;

	private:
		Access access;
		mutable const Pool* iterablePool;


	public:
		TFilter(Access access) : access{Move(access)}, iterablePool{access.GetSmallestPool()} {}

		// Copy of a subset of a query
		// template<typename... Include2, typename... Exclude2>
		// TBasicFilter(const TBasicFilter<TInclude<Include2...>, TExclude<Exclude2...>>& other)
		//{
		//	included = std::tie<TPool<Mut<IncludeComp>>*...>(
		//	    std::get<TPool<Mut<IncludeComp>>*>(other.pools)...);
		//	excludedPools = std::tie<TPool<Mut<ExcludeComp>>*...>(
		//	    std::get<TPool<Mut<ExcludeComp>>*>(other.excluded)...);
		//	iterablePool = GetCandidateIterablePool();
		//}

		Iterator begin() const
		{
			return {iterablePool->begin(), iterablePool->end(), iterablePool->begin(),
			    access.GetPoolArray(iterablePool), access.GetExcludedPoolArray()};
		}

		Iterator end() const
		{
			return {iterablePool->begin(), iterablePool->end(), iterablePool->end(),
			    access.GetPoolArray(iterablePool), access.GetExcludedPoolArray()};
		}

		ReverseIterator rbegin() const
		{
			return {iterablePool->rbegin(), iterablePool->rend(), iterablePool->rbegin(),
			    access.GetPoolArray(iterablePool), access.GetExcludedPoolArray()};
		}

		ReverseIterator rend() const
		{
			return {iterablePool->rbegin(), iterablePool->rend(), iterablePool->rend(),
			    access.GetPoolArray(iterablePool), access.GetExcludedPoolArray()};
		}

		Id Front() const
		{
			const auto it = begin();
			return it != end() ? *it : AST::NoId;
		}

		Id Back() const
		{
			const auto it = rbegin();
			return it != rend() ? *it : AST::NoId;
		}

		Iterator Find(const Id id) const
		{
			const auto it = Iterator{iterablePool->begin(), iterablePool->end(),
			    iterablePool->Find(id), access.GetPoolArray(), access.GetExcludedPoolArray()};
			return (it != end() && *it == id) ? it : end();
		}

		void Each(TFunction<void(Id)> func) const
		{
			auto endIt = end();
			for (auto it = begin(); it != endIt; ++it)
			{
				func(*it);
			}
		}

		void EachBreak(TFunction<bool(Id)> func) const
		{
			auto endIt = end();
			for (auto it = begin(); it != endIt; ++it)
			{
				if (!func(*it))
				{
					break;
				}
			}
		}

		template<typename C>
		void Each(TFunction<void(Id)> func) const
		{
			const Pool* lastIterablePool = iterablePool;
			Use<C>();
			Each(Move(func));
			iterablePool = lastIterablePool;
		}

		template<typename C>
		void EachBreak(TFunction<bool(Id)> func) const
		{
			const Pool* lastIterablePool = iterablePool;
			Use<C>();
			EachBreak(Move(func));
			iterablePool = lastIterablePool;
		}

		bool Has(Id id) const
		{
			return access.Has(id);
		}

		template<typename C>
		bool Has(Id id) const
		{
			return access.Has<C>(id);
		}

		template<typename C>
		decltype(auto) Add(Id id, C&& value = {})
		{
			return access.template GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(Id id, const C& value)
		{
			return access.template GetPool<C>()->Add(id, value);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return access.template GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id)
		{
			return access.template GetPool<C>()->TryGet(id);
		}

		template<typename C>
		const C* TryGet(Id id) const
		{
			return access.template GetPool<C>()->TryGet(id);
		}

		// @brief Forces the type to use to drive iterations
		template<typename C>
		void Use() const
		{
			iterablePool = access.template GetPool<C>();
		}

		i32 Size() const
		{
			return access.Size();
		}

		/**
		 * @return ids matching the filter from another array
		 */
		TArray<Id> FilterIds(const TArray<AST::Id>& ids)
		{
			TArray<Id> filtered;
			for (Id id : ids)
			{
				if (Has(id))
				{
					filtered.Add(id);
				}
			}
			return filtered;
		}

		/**
		 * Removes elements not matching the filter from an array.
		 * order of elements might change.
		 * @see FilterIdsStable
		 */
		void FilterIds(TArray<AST::Id>& ids)
		{
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				if (Has(ids[i]))
				{
					ids.RemoveAtSwap(i, false);
					--i;
				}
			}
			ids.Shrink();
		}

		/**
		 * Removes elements not matching the filter from an array.
		 * Ensures order of elements is kept
		 * @see FilterIds
		 */
		void FilterIdsStable(TArray<AST::Id>& ids)
		{
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				if (Has(ids[i]))
				{
					ids.RemoveAt(i, false);
					--i;
				}
			}
			ids.Shrink();
		}
	};
}    // namespace Rift::AST
