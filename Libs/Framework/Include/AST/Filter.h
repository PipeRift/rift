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


	template<typename...>
	struct TBasicFilter;


	template<typename... IncludeComp, typename... ExcludeComp>
	struct TBasicFilter<TInclude<IncludeComp...>, TExclude<ExcludeComp...>> : public BaseFilter
	{
		using Included = TTypeList<IncludeComp...>;
		using Excluded = TTypeList<ExcludeComp...>;


		template<typename C>
		using Mut = std::remove_const_t<C>;

		using Iterator = Internal::TFilterIterator<Pool::Iterator, sizeof...(IncludeComp) - 1u,
		    sizeof...(ExcludeComp)>;
		using ReverseIterator = Internal::TFilterIterator<Pool::ReverseIterator,
		    sizeof...(IncludeComp) - 1u, sizeof...(ExcludeComp)>;

	private:
		const TTuple<TPool<Mut<IncludeComp>>*...> includedPools;
		const TTuple<TPool<Mut<ExcludeComp>>*...> excludedPools;
		mutable const Pool* iterablePool;


	public:
		TBasicFilter(TTuple<TPool<Mut<IncludeComp>>*...> included,
		    TTuple<TPool<Mut<ExcludeComp>>*...> excluded = {})
		    : includedPools{Move(included)}
		    , excludedPools{Move(excluded)}
		    , iterablePool{GetCandidateIterablePool()}
		{}

		// Copy of a subset of a query
		// template<typename... Include2, typename... Exclude2>
		// TBasicFilter(const TBasicFilter<TInclude<Include2...>, TExclude<Exclude2...>>& other)
		//{
		//	includedPools = std::tie<TPool<Mut<IncludeComp>>*...>(
		//	    std::get<TPool<Mut<IncludeComp>>*>(other.pools)...);
		//	excludedPools = std::tie<TPool<Mut<ExcludeComp>>*...>(
		//	    std::get<TPool<Mut<ExcludeComp>>*>(other.excluded)...);
		//	iterablePool = GetCandidateIterablePool();
		//}

		Iterator begin() const
		{
			return {iterablePool->begin(), iterablePool->end(), iterablePool->begin(),
			    GetSecondaryPoolsArray(), GetExcludedPoolsArray()};
		}

		Iterator end() const
		{
			return {iterablePool->begin(), iterablePool->end(), iterablePool->end(),
			    GetSecondaryPoolsArray(), GetExcludedPoolsArray()};
		}

		ReverseIterator rbegin() const
		{
			return {iterablePool->rbegin(), iterablePool->rend(), iterablePool->rbegin(),
			    GetSecondaryPoolsArray(), GetExcludedPoolsArray()};
		}

		ReverseIterator rend() const
		{
			return {iterablePool->rbegin(), iterablePool->rend(), iterablePool->rend(),
			    GetSecondaryPoolsArray(), GetExcludedPoolsArray()};
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
			    iterablePool->Find(id), GetSecondaryPoolsArray(), GetExcludedPoolsArray()};
			return (it != end() && *it == id) ? it : end();
		}

		template<typename Func>
		void Each(Func func) const
		{
			((GetPool<IncludeComp>(includedPools) == iterablePool
			         ? Traverse<IncludeComp>(Move(func))
			         : void()),
			    ...);
		}

		template<typename C, typename Func>
		void Each(Func func) const
		{
			Use<C>();
			Traverse<C>(Move(func));
		}

		bool Has(Id id) const
		{
			return (GetPool<IncludeComp>()->Has(id) && ...)
			    && (!GetExcluded<ExcludeComp>()->Has(id) && ...);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id)
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		const C* TryGet(Id id) const
		{
			return GetPool<C>()->TryGet(id);
		}

		// @brief Forces the type to use to drive iterations
		template<typename C>
		void Use() const
		{
			iterablePool = GetPool<C>(includedPools);
		}

		i32 Size() const
		{
			static_assert(sizeof...(IncludeComp) == 1 && sizeof...(ExcludeComp) == 0,
			    "Can only get the size of a single component view.");
			return GetPool<IncludeComp...>()->Size();
		}

	protected:
		template<typename T>
		TPool<Mut<T>>* GetPool() const
		{
			return std::get<TPool<Mut<T>>*>(includedPools);
		}
		template<typename T>
		TPool<T>* GetExcluded() const
		{
			return std::get<TPool<Mut<T>>*>(excludedPools);
		}

		const Pool* GetCandidateIterablePool() const
		{
			// Find smallest pool
			return (std::min)({static_cast<const Pool*>(GetPool<IncludeComp>())...},
			    [](const auto* lhs, const auto* rhs) {
				return lhs->Size() < rhs->Size();
			});
		}

		auto GetSecondaryPoolsArray() const
		{
			std::size_t i = 0;
			std::array<const Pool*, sizeof...(IncludeComp) - 1u> other{};

			(static_cast<void>(
			     std::get<TPool<Mut<IncludeComp>>*>(includedPools) == iterablePool
			         ? void()
			         : void(other[i++] = std::get<TPool<Mut<IncludeComp>>*>(includedPools))),
			    ...);
			return other;
		}

		auto GetExcludedPoolsArray() const
		{
			return std::array<const Pool*, sizeof...(ExcludeComp)>{
			    std::get<TPool<ExcludeComp>*>(excludedPools)...};
		}

		template<typename C, typename Func>
		void Traverse(Func func) const
		{
			for (const auto id : *static_cast<const Pool*>(GetPool<C>()))
			{
				if (((IsSame<C, IncludeComp> || GetPool<IncludeComp>()->Has(id)) && ...)
				    && (!GetExcluded<ExcludeComp>()->Has(id) && ...))
				{
					std::apply(func, id);
				}
			}
		}
	};


	//////////////////////////////////////////////////
	// Static filter signatures

	template<typename...>
	struct TFilter;


	template<typename... IncludeComp, typename... ExcludeComp>
	struct TFilter<TInclude<IncludeComp...>, TExclude<ExcludeComp...>>
	    : public TBasicFilter<TInclude<IncludeComp...>, TExclude<ExcludeComp...>>
	{
		// Syntax helpers
		template<typename... T>
		using Include = TFilter<TInclude<IncludeComp..., T...>, TExclude<ExcludeComp...>>;
		template<typename... T>
		using Exclude = TFilter<TInclude<IncludeComp...>, TExclude<ExcludeComp..., T...>>;
	};

	/*template<typename... IncludeComp>
	struct TFilter<TInclude<IncludeComp...>>
	    : public TBasicFilter<TInclude<IncludeComp...>, TExclude<>>
	{
	    // Syntax helpers
	    template<typename... T>
	    using Include = TFilter<JoinList<IncludeComp..., T...>>;
	    template<typename... T>
	    using Exclude = TFilter<TInclude<IncludeComp...>, TExclude<T...>>;
	};

	template<typename... IncludeComp>
	struct TFilter<void, IncludeComp...> : public TBasicFilter<TInclude<IncludeComp...>, TExclude<>>
	{
	    // Syntax helpers
	    template<typename... T>
	    using Include = TFilter<JoinList<IncludeComp..., T...>>;
	    template<typename... T>
	    using Exclude = TFilter<TInclude<IncludeComp...>, TExclude<T...>>;
	};

	template<>
	struct TFilter<> : public TBasicFilter<TInclude<>, TExclude<>>
	{
	    // Syntax helpers
	    template<typename... T>
	    using Include = TFilter<TInclude<T...>>;
	    template<typename... T>
	    using Exclude = TFilter<TInclude<>, TExclude<T...>>;
	};*/
}    // namespace Rift::AST
