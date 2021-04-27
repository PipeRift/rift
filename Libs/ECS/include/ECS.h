// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Misc/Utility.h>

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
		using EnTTView = entt::basic_view<EntityId, Exclude..., Component...>;
		EnTTView view;

	public:
		View(EnTTView view) : view(view) {}
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
			return registry.emplace(entity, Forward<Args>(args)...);
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
		View<TExclude<Exclude...>, Component...> MakeView(TExclude<Exclude...> excluded = {}) const
		{
			return
			{
				registry.view<Component..., Exclude...>(excluded);
			};
		}

		template <typename... Component, typename... Exclude>
		View<TExclude<Exclude...>, Component...> MakeView(TExclude<Exclude...> excluded = {})
		{
			return
			{
				registry.view<Component..., Exclude...>(excluded);
			};
		}
	};
}    // namespace Rift::ECS
