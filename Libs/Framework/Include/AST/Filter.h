// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Pool.h"
#include "AST/Types.h"


namespace Rift::AST
{
	namespace Internal
	{
		template<typename PoolIt, sizet allOf, sizet noneOf>
		class TFilterIterator
		{
			static constexpr sizet i = sizeof(PoolIt);

			PoolIt first;
			PoolIt last;
			PoolIt it;
			std::array<const Pool*, allOf> pools;
			std::array<const Pool*, noneOf> excluded;


		public:
			using iterator_type     = PoolIt;
			using difference_type   = typename std::iterator_traits<PoolIt>::difference_type;
			using value_type        = typename std::iterator_traits<PoolIt>::value_type;
			using pointer           = typename std::iterator_traits<PoolIt>::pointer;
			using reference         = typename std::iterator_traits<PoolIt>::reference;
			using iterator_category = std::bidirectional_iterator_tag;

			TFilterIterator() : first{}, last{}, it{}, pools{}, excluded{} {}

			TFilterIterator(PoolIt from, PoolIt to, PoolIt curr,
			    std::array<const Pool*, allOf> pools, std::array<const Pool*, noneOf> excluded)
			    : first{from}, last{to}, it{curr}, pools{pools}, excluded{excluded}
			{
				if (it != last && !IsValid())
				{
					++(*this);
				}
			}

			TFilterIterator& operator++()
			{
				while (++it != last && !IsValid())
					;
				return *this;
			}

			TFilterIterator operator++(int)
			{
				TFilterIterator orig = *this;
				return ++(*this), orig;
			}

			TFilterIterator& operator--()
			{
				while (--it != first && !IsValid())
					;
				return *this;
			}

			TFilterIterator operator--(int)
			{
				TFilterIterator orig = *this;
				return operator--(), orig;
			}

			[[nodiscard]] bool operator==(const TFilterIterator& other) const
			{
				return other.it == it;
			}

			[[nodiscard]] bool operator!=(const TFilterIterator& other) const
			{
				return !(*this == other);
			}

			[[nodiscard]] pointer operator->() const
			{
				return &*it;
			}

			[[nodiscard]] reference operator*() const
			{
				return *operator->();
			}

		private:
			bool IsValid() const
			{
				const Id id = *it;

				const bool included = std::apply(
				    [id](const auto*... curr) {
					return (curr->Has(id) && ...);
				    },
				    pools);

				return included
				    && std::apply(
				        [id](const auto*... curr) {
					return (!curr->Has(id) && ...);
				        },
				        excluded);
			}
		};
	}    // namespace Internal


	struct BaseFilter
	{
	private:
		mutable const Pool* iterablePool;
	};


	template<typename Access>
	struct TFilter : public BaseFilter
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
			    iterablePool->Find(id), GetPoolsArray(), GetExcludedPoolsArray()};
			return (it != end() && *it == id) ? it : end();
		}

		/*template<typename Func>
		void Each(Func func) const
		{
		    ((access.GetPool<IncludeComp>(included) == iterablePool
		             ? Traverse<IncludeComp>(Move(func))
		             : void()),
		        ...);
		}*/

		template<typename C, typename Func>
		void Each(Func func) const
		{
			Use<C>();
			Traverse<C>(Move(func));
		}

		bool Has(Id id) const
		{
			return access.Has(id);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return access.GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id)
		{
			return access.GetPool<C>()->TryGet(id);
		}

		template<typename C>
		const C* TryGet(Id id) const
		{
			return access.GetPool<C>()->TryGet(id);
		}

		// @brief Forces the type to use to drive iterations
		template<typename C>
		void Use() const
		{
			iterablePool = access.GetPool<C>();
		}

		i32 Size() const
		{
			return access.Size();
		}

	protected:
		/*template<typename C, typename Func>
		void Traverse(Func func) const
		{
		    for (const auto id : *static_cast<const Pool*>(access.GetPool<C>()))
		    {
		        if (((IsSame<C, IncludeComp> || access.GetPool<IncludeComp>()->Has(id)) && ...)
		            && (!access.GetExcluded<ExcludeComp>()->Has(id) && ...))
		        {
		            std::apply(func, id);
		        }
		    }
		}*/
	};
}    // namespace Rift::AST
