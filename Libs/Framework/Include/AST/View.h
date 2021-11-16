// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <entt/entity/view.hpp>


namespace Rift::AST
{
	namespace Internal
	{
		template<sizet AllOf, sizet NoneOf>
		class ViewIterator
		{
			using PoolIt       = BasePool::Iterator;
			static constexpr i = sizeof(PoolIt);

			bool IsValid() const
			{
				const Id id = *it;
				return std::all_of(
				           [id](const auto*... curr) {
					return (curr->contains(entt) && ...);
				           },
				           pools)
				    && std::none_of(
				        [id](const auto*... curr) {
					return (curr->contains(entt) && ...);
				        },
				        filter);
			}

		public:
			using iterator_type     = It;
			using difference_type   = typename std::iterator_traits<It>::difference_type;
			using value_type        = typename std::iterator_traits<It>::value_type;
			using pointer           = typename std::iterator_traits<It>::pointer;
			using reference         = typename std::iterator_traits<It>::reference;
			using iterator_category = std::bidirectional_iterator_tag;

			ViewIterator() : first{}, last{}, it{}, pools{}, filter{} {}

			ViewIterator(It from, It to, It curr, std::array<const BasePool*, AllOf> all_of,
			    std::array<const BasePool*, NoneOf> noneOf)
			    : first{from}, last{to}, it{curr}, pools{all_Of}, filter{noneOf}
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

			Iterator operator++(int)
			{
				Iterator orig = *this;
				return ++(*this), orig;
			}

			Iterator& operator--()
			{
				while (--it != first && !IsValid())
					;
				return *this;
			}

			Iterator operator--(int)
			{
				Iterator orig = *this;
				return operator--(), orig;
			}

			[[nodiscard]] bool operator==(const Iterator& other) const
			{
				return other.it == it;
			}

			[[nodiscard]] bool operator!=(const Iterator& other) const
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
			It first;
			It last;
			It it;
			std::array<const BasePool*, AllOf> pools;
			std::array<const BasePool*, NoneOf> filter;
		};
	}    // namespace Internal


	struct BaseView
	{
	private:
		const TArray<const BasePool*> pools;
		const TArray<const BasePool*> excluded;
		mutable const BasePool* iterablePool;
	};


	template<typename...>
	struct TView;


	template<typename... Exclude, typename... Component>
	struct TView<TExclude<Exclude...>, Component...> : public BaseView
	{
	private:
		const TTuple<const TPool<Component>*...> pools;
		const TTuple<const TPool<Exclude>*...> excluded;
		mutable const BasePool* iterablePool;


	public:
		TView(const TPool<Component>&... pools, const TPool<Component>&... excluded)
		    : pools{&pools...}, excluded{&excluded...}, iterablePool{GetCandidateIterablePool()}
		{}

		Iterator begin() const
		{
			return view.begin();
		}

		Iterator end() const
		{
			return view.end();
		}

		ReverseIterator rbegin() const
		{
			return view.rbegin();
		}

		ReverseIterator rend() const
		{
			return view.rend();
		}

		Id Front() const
		{
			return view.front();
		}

		Id Back() const
		{
			return view.back();
		}

		Iterator Find(const Id id) const
		{
			return view.find(id);
		}

		template<typename Func>
		void Each(Func func) const
		{
			view.each(func);
		}

		template<typename Comp, typename Func>
		void Each(Func func) const
		{
			view.template each<Comp>(func);
		}

		bool Has(const Id id) const
		{
			return (GetPool<Component>()->Has(id) && ...)
			    && (!GetExcluded<Exclude>()->Has(id) && ...);
		}

		template<typename C>
		C& Get(const Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(const Id node)
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		const C* TryGet(const Id node) const
		{
			return GetPool<C>()->TryGet(id);
		}

		i32 Size() const
		{
			static_assert(sizeof...(Component) == 1 && sizeof...(Exclude) == 0,
			    "Can only get the size of a single component view.");
			return GetPool<Component>()->Size();
		}

	private:
		template<typename T>
		TPool<T>* GetPool()
		{
			return std::get<TPool<T>*>(pools);
		}
		template<typename T>
		TPool<T>* GetExcluded()
		{
			return std::get<TPool<T>*>(excluded);
		}

		const BasePool* GetCandidateIterablePool() const
		{
			// Find smallest pool
			return (std::min)({static_cast<const BasePool*>(GetPool<Component>())...},
			    [](const auto* lhs, const auto* rhs) {
				return lhs->Size() < rhs->Size();
			});
		}
	};
}    // namespace Rift::AST
