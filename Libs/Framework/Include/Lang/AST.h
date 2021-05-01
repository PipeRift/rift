// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/AST/ASTTypes.h"
#include "Lang/AST/View.h"

#include <Strings/Name.h>

#include <entt/entt.hpp>


namespace Rift::AST
{
	struct AbstractSyntaxTree
	{
	private:
		entt::registry registry;


	public:
		// This helpers simplify the creation of language elements
		Id CreateClass(Name name);
		Id CreateStruct(Name name);
		Id CreateVariable(Name name);
		Id CreateFunction(Name name);


		void AddChild(Id parent, Id child);
		void RemoveChild(Id parent, Id child);


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
		decltype(auto) GetComponentsPtr(const Id node)
		{
			return registry.try_get<Component...>(node);
		}

		template <typename Component>
		const auto& GetComponent(const Id node) const
		{
			return GetComponents<Component>(node);
		}

		template <typename Component>
		auto& GetComponent(const Id node)
		{
			return GetComponents<Component>(node);
		}

		template <typename Component>
		const auto* GetComponentPtr(const Id node) const
		{
			return GetComponentsPtr<Component>(node);
		}

		template <typename Component>
		auto* GetComponentPtr(const Id node)
		{
			return GetComponentsPtr<Component>(node);
		}

		template <typename Component, typename... Args>
		auto& GetOrAddComponent(const Id node, Args&&... args)
		{
			return registry.get_or_emplace<Component>(node, Forward<Args>(args)...);
		}


		template <typename... Component>
		bool HasAnyComponents(Id node) const
		{
			return registry.any_of<Component...>(node);
		}

		template <typename... Component>
		bool HasAllComponents(Id node) const
		{
			return registry.all_of<Component...>(node);
		}

		template <typename Component>
		bool HasComponent(Id node) const
		{
			return HasAnyComponents<Component>(node);
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
#pragma endregion ECS API

		void RemoveChildFromCChildren(Id parent, Id child);
	};
}    // namespace Rift::AST
