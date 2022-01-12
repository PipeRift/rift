// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Pool.h"
#include "AST/Types.h"

#include <Containers/Array.h>
#include <Reflection/TypeId.h>
#include <Templates/Tuples.h>
#include <Templates/TypeList.h>


namespace Rift::AST
{
	template<typename PoolIt>
	class TAccessIterator
	{
		static constexpr sizet i = sizeof(PoolIt);

		PoolIt first;
		PoolIt last;
		PoolIt it;
		TArray<Pool*> pools;
		TArray<const Pool*> excluded;


	public:
		using iterator_type     = PoolIt;
		using difference_type   = typename std::iterator_traits<PoolIt>::difference_type;
		using value_type        = typename std::iterator_traits<PoolIt>::value_type;
		using pointer           = typename std::iterator_traits<PoolIt>::pointer;
		using reference         = typename std::iterator_traits<PoolIt>::reference;
		using iterator_category = std::bidirectional_iterator_tag;

		TAccessIterator() : first{}, last{}, it{}, pools{}, excluded{} {}

		TAccessIterator(PoolIt from, PoolIt to, PoolIt curr, TArray<Pool*>&& pools,
		    TArray<const Pool*>&& excluded)
		    : first{from}, last{to}, it{curr}, pools{Move(pools)}, excluded{Move(excluded)}
		{
			if (it != last && !IsValid())
			{
				++(*this);
			}
		}

		TAccessIterator& operator++()
		{
			while (++it != last && !IsValid()) {}
			return *this;
		}

		TAccessIterator operator++(int)
		{
			TAccessIterator orig = *this;
			return ++(*this), orig;
		}

		TAccessIterator& operator--()
		{
			while (--it != first && !IsValid()) {}
			return *this;
		}

		TAccessIterator operator--(int)
		{
			TAccessIterator orig = *this;
			return operator--(), orig;
		}

		[[nodiscard]] bool operator==(const TAccessIterator& other) const
		{
			return other.it == it;
		}

		[[nodiscard]] bool operator!=(const TAccessIterator& other) const
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
			if (AST::GetVersion(id) == AST::GetVersion(AST::NoId))
			{
				return false;
			}
			for (const Pool* pool : pools)
			{
				if (!pool->Has(id))
				{
					return false;
				}
			}
			for (const Pool* pool : excluded)
			{
				if (pool->Has(id))
				{
					return false;
				}
			}
			return true;
		}
	};


	template<typename... T>
	struct TAccess;

	template<typename... IncludeComp, typename... ExcludeComp>
	struct TAccess<TInclude<IncludeComp...>, TExclude<ExcludeComp...>>
	{
		template<typename... K>
		friend struct TAccess;

		static_assert(sizeof...(IncludeComp) > 0, "Exclusion-only access are not supported");

		using Iterator        = TAccessIterator<Pool::Iterator>;
		using ReverseIterator = TAccessIterator<Pool::ReverseIterator>;

		using Included = TTypeList<IncludeComp...>;
		using Excluded = TTypeList<ExcludeComp...>;

		using IncludedTuple = TTuple<TPool<Mut<IncludeComp>>*...>;
		using ExcludedTuple = TTuple<const TPool<Mut<ExcludeComp>>*...>;


		template<typename... T>
		using In = TAccess<TInclude<IncludeComp..., T...>, TExclude<ExcludeComp...>>;
		template<typename... T>
		using Ex = TAccess<TInclude<ExcludeComp...>, TExclude<ExcludeComp..., T...>>;


	protected:
		IncludedTuple included;
		ExcludedTuple excluded;


	public:
		TAccess(IncludedTuple included, ExcludedTuple excluded)
		    : included{included}, excluded{excluded}
		{}

		/**
		 * Creates a superset access from other access
		 * Access A can be created from access A|B
		 */
		template<typename... OtherIncludeComp, typename... OtherExcludeComp>
		TAccess(const TAccess<TInclude<OtherIncludeComp...>, TExclude<OtherExcludeComp...>>& other)
		    : included{std::get<TPool<Mut<IncludeComp>>*>(other.included)...}
		    , excluded{std::get<const TPool<Mut<ExcludeComp>>*>(other.excluded)...}
		{}

		Pool* GetSmallestPool() const
		{
			// Find smallest pool
			return (std::min)({static_cast<Pool*>(GetPool<IncludeComp>())...},
			    [](const auto* lhs, const auto* rhs) {
				return lhs->Size() < rhs->Size();
			});
		}

		bool Has(Id id) const
		{
			return id != AST::NoId && (GetPool<IncludeComp>()->Has(id) && ...)
			    && (!GetExcludedPool<ExcludeComp>()->Has(id) && ...);
		}

		template<typename T>
		bool Has(Id id) const
		{
			return GetPool<T>()->Has(id);
		}

		// Get the size of an access to one single component
		i32 Size() const
		{
			static_assert(sizeof...(IncludeComp) == 1 && sizeof...(ExcludeComp) == 0,
			    "Can only get the size from single component access.");
			return GetPool<IncludeComp...>()->Size();
		}

		template<typename T>
		TPool<Mut<T>>* GetPool() const
		{
			if constexpr (Contains<TPool<Mut<T>>*, decltype(included)>())
			{
				return std::get<TPool<Mut<T>>*>(included);
			}
			return nullptr;
		}

		template<typename T>
		const TPool<T>* GetExcludedPool() const
		{
			if constexpr (Contains<const TPool<Mut<T>>*, decltype(excluded)>())
			{
				return std::get<const TPool<Mut<T>>*>(excluded);
			}
			return nullptr;
		}

		TArray<Pool*> GetPoolArray(const Pool* ignoredPool = nullptr) const
		{
			if (!ignoredPool)
			{
				return {GetPool<IncludeComp>()...};
			}
			TArray<Pool*> pools{};
			pools.Reserve(sizeof...(IncludeComp) - 1u);
			(static_cast<void>(GetPool<IncludeComp>() != ignoredPool
			                       ? void(pools.Add(GetPool<IncludeComp>()))
			                       : void()),
			    ...);
			return Move(pools);
		}

		TArray<const Pool*> GetExcludedPoolArray() const
		{
			return {GetExcludedPool<ExcludeComp>()...};
		}
	};

	struct Access
	{
		template<typename... T>
		using In = TAccess<TInclude<T...>, TExclude<>>;
	};

	/*struct RuntimeAccess
	{
	    struct PoolPtr
	    {
	        Refl::TypeId id;
	        Pool* pool = nullptr;

	        struct Sort
	        {
	            constexpr bool operator()(const PoolPtr& a, const PoolPtr& b) const
	            {
	                return a.id < b.id;
	            }

	            constexpr bool operator()(Refl::TypeId a, const PoolPtr& b) const
	            {
	                return a < b.id;
	            }

	            constexpr bool operator()(const PoolPtr& a, Refl::TypeId b) const
	            {
	                return a.id < b;
	            }
	        };
	    };

	protected:

	    TArray<PoolPtr> included;
	    TArray<PoolPtr> excluded;


	public:
	    RuntimeAccess() {}

	    i32 AddInclude(Refl::TypeId id)
	    {
	        return included.FindOrAddSorted(PoolPtr{id}, PoolPtr::Sort{}).first;
	    }
	    i32 AddExclude(Refl::TypeId id)
	    {
	        return excluded.FindOrAddSorted(PoolPtr{id}, PoolPtr::Sort{}).first;
	    }

	    bool RemoveInclude(Refl::TypeId id)
	    {
	        return included.RemoveSorted(PoolPtr{id}) > 0;
	    }
	    bool RemoveExclude(Refl::TypeId id)
	    {
	        return excluded.RemoveSorted(PoolPtr{id}) > 0;
	    }

	    template<typename T>
	    TPool<Mut<T>>* GetPool() const
	    {
	        const u32 index = included.FindSortedEqual(TypeId::Get<T>(), PoolPtr::Sort{});
	        return index != NO_INDEX ? static_cast<TPool<Mut<T>>*>(included[index].pool) : nullptr;
	    }

	    template<typename T>
	    TPool<Mut<T>>* GetExcludedPool() const
	    {
	        const u32 index = excluded.FindSortedEqual(TypeId::Get<T>(), PoolPtr::Sort{});
	        return index != NO_INDEX ? static_cast<TPool<Mut<T>>*>(excluded[index].pool) : nullptr;
	    }

	    Pool* GetSmallestPool() const
	    {
	        return nullptr;
	    }
	};*/
}    // namespace Rift::AST
