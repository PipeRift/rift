// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Misc/Utility.h>

#include <entt/entity/view.hpp>
#include <entt/entt.hpp>


namespace Rift::ECS
{
	using EntityId    = entt::entity;
	using VersionType = entt::entt_traits<EntityId>::version_type;

	template <typename... Type>
	using TExclude = entt::exclude_t<Type...>;

	template <typename...>
	struct View;

	template <typename... Exclude, typename... Component>
	struct View<TExclude<Exclude...>, Component...>
	{
	private:
		using EnTTView        = entt::basic_view<EntityId, TExclude<Exclude...>, Component...>;
		using Iterator        = typename EnTTView::iterator;
		using ReverseIterator = typename EnTTView::reverse_iterator;

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

		EntityId Front() const
		{
			return view.front();
		}

		EntityId Back() const
		{
			return view.back();
		}

		Iterator Find(const EntityId entity) const
		{
			return view.find(entt);
		}

		template <typename Func>
		void Each(Func func) const
		{
			view.Each(func);
		}

		template <typename Comp, typename Func>
		void Each(Func func) const
		{
			view.Each<Comp>(func);
		}

		bool Contains(const EntityId entity) const
		{
			return view.contains(entity);
		}

		template <typename... Comp>
		decltype(auto) Get([[maybe_unused]] const EntityId entity) const
		{
			return view.get<Comp...>(entity);
		}
	};


	struct Registry
	{
		entt::basic_registry<EntityId> registry;


		EntityId Create()
		{
			return registry.create();
		}

		EntityId Create(const EntityId hint)
		{
			return registry.create(Move(hint));
		}

		void Destroy(const EntityId entity)
		{
			registry.destroy(entity);
		}

		void Destroy(const EntityId entity, const VersionType version)
		{
			registry.destroy(entity, version);
		}

		template <typename Component, typename... Args>
		auto Emplace(EntityId entity, Args&&... args)
		{
			return registry.emplace<Component>(entity, Forward<Args>(args)...);
		}

		template <typename... Component>
		void Remove(const EntityId entity)
		{
			registry.remove<Component...>(entity);
		}

		void RemoveAll(const EntityId entity)
		{
			registry.remove_all(entity);
		}

		template <typename... Component>
		auto Get(const EntityId entity) const
		{
			return registry.get<Component...>(entity);
		}

		template <typename... Component>
		auto Get(const EntityId entity)
		{
			return registry.get<Component...>(entity);
		}

		template <typename... Component>
		auto GetPtr(const EntityId entity) const
		{
			return registry.try_get<Component...>(entity);
		}

		template <typename... Component>
		auto GetPtr(const EntityId entity)
		{
			return registry.try_get<Component...>(entity);
		}

		template <typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {}) const
		{
			return View<TExclude<Exclude...>, Component...>{registry.view<Component...>(excluded)};
		}

		template <typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {})
		{
			return View<TExclude<Exclude...>, Component...>{registry.view<Component...>(excluded)};
		}
	};
}    // namespace Rift::ECS
