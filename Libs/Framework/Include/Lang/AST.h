// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/AST/ASTTypes.h"
#include "Lang/AST/View.h"
#include "Lang/CChildren.h"
#include "Lang/CParent.h"

#include <Strings/Name.h>

#include <entt/entt.hpp>


namespace Rift::AST
{
	struct Tree
	{
	private:
		entt::basic_registry<Id> registry;

	public:
		View<TExclude<>, CParent> parentView;
		View<TExclude<>, CChildren> childrenView;


	public:
		Tree() : registry{}, parentView(MakeView<CParent>()), childrenView(MakeView<CChildren>()) {}

#pragma region ECS API
		Id Create();
		Id Create(const Id hint);
		void Destroy(const Id node);
		void Destroy(const Id node, const VersionType version);

		/**
		 * Adds Component to an entity (if the entity doesnt have it already)
		 */
		template <typename Component, typename... Args>
		decltype(auto) AddComponent(Id node, Args&&... args)
		{
			return registry.emplace<Component>(node, Forward<Args>(args)...);
		}

		template <typename... Component>
		void AddComponents(Id node)
		{
			(AddComponent<Component>(node), ...);
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
		decltype(auto) EmplaceComponent(Id node, Args&&... args)
		{
			return registry.emplace_or_replace<Component>(node, Forward<Args>(args)...);
		}

		template <typename Component>
		void RemoveComponent(const Id node)
		{
			RemoveComponents<Component>(node);
		}

		template <typename... Component>
		void RemoveComponents(const Id node)
		{
			registry.remove<Component...>(node);
		}

		void RemoveAllComponents(const Id node)
		{
			registry.remove_all(node);
		}

		template <typename... Component>
		decltype(auto) GetComponents(const Id node) const
		{
			return registry.get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) GetComponents(const Id node)
		{
			return registry.get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) GetComponentsPtr(const Id node) const
		{
			return registry.try_get<Component...>(node);
		}

		template <typename... Component>
		decltype(auto) GetComponentPtrs(const Id node)
		{
			return registry.try_get<Component...>(node);
		}

		template <typename Component>
		auto GetComponent(const Id node) const
		{
			return GetComponents<Component>(node);
		}

		template <typename Component>
		auto GetComponent(const Id node)
		{
			return GetComponents<Component>(node);
		}

		template <typename Component>
		const auto* GetComponentPtr(const Id node) const
		{
			return GetComponentPtrs<Component>(node);
		}

		template <typename Component>
		auto* GetComponentPtr(const Id node)
		{
			return GetComponentPtrs<Component>(node);
		}

		template <typename Component, typename... Args>
		auto& GetOrAddComponent(const Id node, Args&&... args)
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
#pragma endregion ECS API

		void RemoveChildFromCChildren(Id parent, Id child);
	};
}    // namespace Rift::AST
