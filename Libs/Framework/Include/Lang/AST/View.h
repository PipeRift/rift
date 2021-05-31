// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/AST/ASTTypes.h"

#include <entt/entity/view.hpp>


namespace Rift::AST
{
	template <typename...>
	struct View;


	template <typename... Exclude, typename... Component>
	struct View<TExclude<Exclude...>, Component...>
	{
		using EnTTView        = entt::basic_view<Id, TExclude<Exclude...>, Component...>;
		using Iterator        = typename EnTTView::iterator;
		using ReverseIterator = typename EnTTView::reverse_iterator;

	private:
		EnTTView view;


	public:
		View(EnTTView view) : view(view) {}

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

		Iterator Find(const Id node) const
		{
			return view.find(node);
		}

		template <typename Func>
		void Each(Func func) const
		{
			view.each(func);
		}

		template <typename Comp, typename Func>
		void Each(Func func) const
		{
			view.template each<Comp>(func);
		}

		bool Has(const Id node) const
		{
			return view.contains(node);
		}

		template <typename... Comp>
		decltype(auto) Get(const Id node) const
		{
			return view.get<Comp...>(node);
		}

		template <typename Comp>
		Comp* TryGet(const Id node)
		{
			if (Has(node))
			{
				return &view.get<Comp>(node);
			}
			return nullptr;
		}

		template <typename Comp>
		const Comp* TryGet(const Id node) const
		{
			if (Has(node))
			{
				return &view.get<Comp>(node);
			}
			return nullptr;
		}
	};
}    // namespace Rift::AST
