// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Pool.h"
#include "AST/Types.h"


namespace Rift::AST
{
	namespace Internal
	{
		template<typename PoolIt, sizet allOf, sizet noneOf>
		class ViewIterator
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

			ViewIterator() : first{}, last{}, it{}, pools{}, excluded{} {}

			ViewIterator(PoolIt from, PoolIt to, PoolIt curr, std::array<const Pool*, allOf> pools,
			    std::array<const Pool*, noneOf> excluded)
			    : first{from}, last{to}, it{curr}, pools{pools}, excluded{excluded}
			{
				if (it != last && !IsValid())
				{
					++(*this);
				}
			}

			ViewIterator& operator++()
			{
				while (++it != last && !IsValid())
					;
				return *this;
			}

			ViewIterator operator++(int)
			{
				ViewIterator orig = *this;
				return ++(*this), orig;
			}

			ViewIterator& operator--()
			{
				while (--it != first && !IsValid())
					;
				return *this;
			}

			ViewIterator operator--(int)
			{
				ViewIterator orig = *this;
				return operator--(), orig;
			}

			[[nodiscard]] bool operator==(const ViewIterator& other) const
			{
				return other.it == it;
			}

			[[nodiscard]] bool operator!=(const ViewIterator& other) const
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


	struct BaseView
	{
	private:
		mutable const Pool* iterablePool;
	};


	template<typename...>
	struct TQuery;


	template<typename... Exclude, typename... Component>
	struct TQuery<TExclude<Exclude...>, Component...> : public BaseView
	{
		template<typename C>
		using Mut = std::remove_const_t<C>;

		using Iterator =
		    Internal::ViewIterator<Pool::Iterator, sizeof...(Component) - 1u, sizeof...(Exclude)>;
		using ReverseIterator = Internal::ViewIterator<Pool::ReverseIterator,
		    sizeof...(Component) - 1u, sizeof...(Exclude)>;

	private:
		const TTuple<TPool<Mut<Component>>*...> pools;
		const TTuple<TPool<Exclude>*...> excluded;
		mutable const Pool* iterablePool;


	public:
		TQuery(TTuple<TPool<Mut<Component>>*...> pools, TTuple<TPool<Exclude>*...> excluded = {})
		    : pools{pools}, excluded{excluded}, iterablePool{GetCandidateIterablePool()}
		{}

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
			((GetPool<Component>(pools) == iterablePool ? Traverse<Component>(Move(func)) : void()),
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
			return (GetPool<Component>()->Has(id) && ...)
			    && (!GetExcluded<Exclude>()->Has(id) && ...);
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
			iterablePool = GetPool<C>(pools);
		}

		i32 Size() const
		{
			static_assert(sizeof...(Component) == 1 && sizeof...(Exclude) == 0,
			    "Can only get the size of a single component view.");
			return GetPool<Component...>()->Size();
		}

	private:
		template<typename T>
		TPool<Mut<T>>* GetPool() const
		{
			return std::get<TPool<Mut<T>>*>(pools);
		}
		template<typename T>
		TPool<T>* GetExcluded() const
		{
			return std::get<TPool<Mut<T>>*>(excluded);
		}

		const Pool* GetCandidateIterablePool() const
		{
			// Find smallest pool
			return (std::min)({static_cast<const Pool*>(GetPool<Component>())...},
			    [](const auto* lhs, const auto* rhs) {
				return lhs->Size() < rhs->Size();
			});
		}

		auto GetSecondaryPoolsArray() const
		{
			std::size_t i = 0;
			std::array<const Pool*, sizeof...(Component) - 1u> other{};

			(static_cast<void>(std::get<TPool<Mut<Component>>*>(pools) == iterablePool
			                       ? void()
			                       : void(other[i++] = std::get<TPool<Mut<Component>>*>(pools))),
			    ...);
			return other;
		}

		auto GetExcludedPoolsArray() const
		{
			return std::array<const Pool*, sizeof...(Exclude)>{
			    std::get<TPool<Exclude>*>(excluded)...};
		}

		template<typename C, typename Func>
		void Traverse(Func func) const
		{
			for (const auto id : *static_cast<const Pool*>(GetPool<C>()))
			{
				if (((IsSame<C, Component> || GetPool<Component>()->Has(id)) && ...)
				    && (!GetExcluded<Exclude>()->Has(id) && ...))
				{
					std::apply(func, id);
				}
			}
		}
	};
}    // namespace Rift::AST
