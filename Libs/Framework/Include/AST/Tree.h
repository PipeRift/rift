// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "AST/View.h"
#include "AST/Entt/RegistryTraits.h"

#include <Strings/Name.h>

#include <entt/entity/registry.hpp>


namespace Rift
{
	struct CChild;
	struct CParent;
}    // namespace Rift


namespace Rift::AST
{
	struct Tree
	{
		using Registry = entt::basic_registry<Id>;
		using Pool     = Registry::poly_storage;

	private:
		Registry registry;
		Pool* childPool;
		Pool* parentPool;


	public:
		Tree();
		Tree(const Tree& other) = delete;
		Tree& operator=(const Tree& other) = delete;
		Tree(Tree&& other);
		Tree& operator=(Tree&& other);

#pragma region ECS API
		Id Create();
		Id Create(const Id hint);
		template <typename It>
		void Create(It first, It last)
		{
			registry.create(first, last);
		}
		void Destroy(const Id node);
		void Destroy(const Id node, const VersionType version);
		template <typename It>
		void Destroy(It first, It last)
		{
			registry.destroy(first, last);
		}

		/**
		 * Adds Component to an entity (if the entity doesnt have it already)
		 */
		template <typename Component, typename... Args>
		decltype(auto) Add(Id node, Args&&... args)
		{
			return registry.emplace<Component>(node, Forward<Args>(args)...);
		}

		template <typename... Components>
		void Add(Id node) requires(sizeof...(Components) > 1)
		{
			(Add<Components>(node), ...);
		}

		/**
		 * If the entity has Component, it will be replaced)
		 */
		template <typename Component, typename... Args>
		decltype(auto) ReplaceComponent(Id node, Args&&... args)
		{
			return registry.replace<Component>(node, Forward<Args>(args)...);
		}

		/**
		 * Adds Component to an entity (if the entity already has it, it will be replaced)
		 */
		template <typename Component, typename... Args>
		decltype(auto) Emplace(Id node, Args&&... args)
		{
			return registry.emplace_or_replace<Component>(node, Forward<Args>(args)...);
		}

		template <typename... Component>
		void Remove(const Id node)
		{
			registry.remove<Component...>(node);
		}

		void RemoveAll(const Id node)
		{
			registry.remove_all(node);
		}

		template <typename... Component>
		decltype(auto) Get(const Id node) const
		{
			return registry.get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) Get(const Id node)
		{
			return registry.get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) TryGet(const Id node) const
		{
			return registry.try_get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) TryGet(const Id node)
		{
			return registry.try_get<Component...>(node);
		}

		template <typename Component, typename... Args>
		auto& GetOrAdd(const Id node, Args&&... args)
		{
			return registry.get_or_emplace<Component>(node, Forward<Args>(args)...);
		}


		template <typename... Component>
		bool HasAny(Id node) const
		{
			return registry.any_of<Component...>(node);
		}

		template <typename... Component>
		bool HasAll(Id node) const
		{
			return registry.all_of<Component...>(node);
		}

		template <typename Component>
		bool Has(Id node) const
		{
			return HasAny<Component>(node);
		}

		bool IsValid(Id node) const
		{
			return registry.valid(node);
		}


		template <typename Component, typename... Args>
		Component& SetUnique(Args&&... args)
		{
			return registry.set<Component>(std::forward<Args>(args)...);
		}

		template <typename Component, typename... Args>
		Component& GetOrSetUnique(Args&&... args)
		{
			if (Component* existing = TryGetUnique<Component>())
			{
				return *existing;
			}
			return SetUnique<Component>(std::forward<Args>(args)...);
		}

		template <typename Component>
		const Component& GetUnique() const
		{
			return registry.ctx<const Component>();
		}

		template <typename Component>
		Component& GetUnique()
		{
			return registry.ctx<Component>();
		}

		template <typename Component>
		const Component* TryGetUnique() const
		{
			return registry.try_ctx<const Component>();
		}

		template <typename Component>
		Component* TryGetUnique()
		{
			return registry.try_ctx<Component>();
		}


		template <typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {}) const
		    -> View<TExclude<Exclude...>, Component...>
		{
			return View<TExclude<Exclude...>, Component...>{registry.view<Component...>(excluded)};
		}

		template <typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {})
		    -> View<TExclude<Exclude...>, Component...>
		{
			return View<TExclude<Exclude...>, Component...>{registry.view<Component...>(excluded)};
		}

		template <typename Callback>
		void Each(Callback cb) const
		{
			registry.each(cb);
		}

		template <typename Callback>
		void EachOrphan(Callback cb) const
		{
			registry.orphans(cb);
		}

		template <typename... Components>
		void Clear()
		{
			registry.clear<Components...>();
		}

		void Reset()
		{
			registry = {};
		}

		Pool& GetParentPool()
		{
			return *parentPool;
		}

		Pool& GetChildPool()
		{
			return *childPool;
		}

		const Pool& GetParentPool() const
		{
			return *parentPool;
		}

		const Pool& GetChildPool() const
		{
			return *childPool;
		}
#pragma endregion ECS API

		void CopyFrom(const Tree& other);

	private:
		void SetupNativeTypes();
		void CachePools();

	};
}    // namespace Rift::AST
