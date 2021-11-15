// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Components/CChild.h"
#include "AST/Components/CParent.h"
#include "AST/Entt/RegistryTraits.h"
#include "AST/IdRegistry.h"
#include "AST/Pool.h"
#include "AST/Types.h"
#include "AST/View.h"

#include <Memory/UniquePtr.h>
#include <Strings/Name.h>

#include <entt/entity/registry.hpp>


namespace Rift::AST
{
	struct NativeTypeIds
	{
		AST::Id boolId   = AST::NoId;
		AST::Id floatId  = AST::NoId;
		AST::Id doubleId = AST::NoId;
		AST::Id u8Id     = AST::NoId;
		AST::Id i8Id     = AST::NoId;
		AST::Id u16Id    = AST::NoId;
		AST::Id i16Id    = AST::NoId;
		AST::Id u32Id    = AST::NoId;
		AST::Id i32Id    = AST::NoId;
		AST::Id u64Id    = AST::NoId;
		AST::Id i64Id    = AST::NoId;
		AST::Id stringId = AST::NoId;
	};

	struct Tree
	{
		using Registry = entt::basic_registry<Id>;
		using Pool     = Registry::poly_storage;

	private:
		IdRegistry idRegistry;
		Registry registry;
		TOwnPtr<View<TExclude<>, CChild>> childView;
		TOwnPtr<View<TExclude<>, CParent>> parentView;
		NativeTypeIds nativeTypes;

		TArray<PoolInstance> pools;


	public:
		Tree();
		Tree(const Tree& other) = delete;
		Tree& operator=(const Tree& other) = delete;
		Tree(Tree&& other);
		Tree& operator=(Tree&& other);

#pragma region ECS API
		Id Create();
		void Create(Id id);
		void Create(TArrayView<Id> ids);
		void Destroy(Id id);
		void Destroy(TArrayView<const Id> ids);

		/**
		 * Adds Component to an entity (if the entity doesnt have it already)
		 */
		template<typename Component, typename... Args>
		decltype(auto) Add(Id node, Args&&... args)
		{
			return registry.emplace<Component>(node, Forward<Args>(args)...);
		}

		template<typename... Components>
		void Add(Id node) requires(sizeof...(Components) > 1)
		{
			(Add<Components>(node), ...);
		}

		/**
		 * If the entity has Component, it will be replaced
		 */
		template<typename Component, typename... Args>
		decltype(auto) Replace(Id node, Args&&... args)
		{
			return registry.replace<Component>(node, Forward<Args>(args)...);
		}

		/**
		 * Adds Component to an entity (if the entity already has it, it will be replaced)
		 */
		template<typename Component, typename... Args>
		decltype(auto) Emplace(Id node, Args&&... args)
		{
			return registry.emplace_or_replace<Component>(node, Forward<Args>(args)...);
		}

		template<typename... Component>
		void Remove(const Id node)
		{
			registry.remove<Component...>(node);
		}

		void RemoveAll(const Id node)
		{
			registry.remove_all(node);
		}

		template<typename... Component>
		decltype(auto) Get(const Id node) const
		{
			return registry.get<Component...>(node);
		}

		template<typename... Component>
		decltype(auto) Get(const Id node)
		{
			return registry.get<Component...>(node);
		}

		template<typename... Component>
		decltype(auto) TryGet(const Id node) const
		{
			return registry.try_get<Component...>(node);
		}

		template<typename... Component>
		decltype(auto) TryGet(const Id node)
		{
			return registry.try_get<Component...>(node);
		}

		template<typename Component, typename... Args>
		auto& GetOrAdd(const Id node, Args&&... args)
		{
			return registry.get_or_emplace<Component>(node, Forward<Args>(args)...);
		}


		template<typename... Component>
		bool HasAny(Id node) const
		{
			return registry.any_of<Component...>(node);
		}

		template<typename... Component>
		bool HasAll(Id node) const
		{
			return registry.all_of<Component...>(node);
		}

		template<typename Component>
		bool Has(Id node) const
		{
			return HasAny<Component>(node);
		}

		bool IsValid(Id node) const
		{
			return registry.valid(node);
		}


		template<typename Component, typename... Args>
		Component& SetUnique(Args&&... args)
		{
			return registry.set<Component>(std::forward<Args>(args)...);
		}

		template<typename Component, typename... Args>
		Component& GetOrSetUnique(Args&&... args)
		{
			if (Component* existing = TryGetUnique<Component>())
			{
				return *existing;
			}
			return SetUnique<Component>(std::forward<Args>(args)...);
		}

		template<typename Component>
		const Component& GetUnique() const
		{
			return registry.ctx<const Component>();
		}

		template<typename Component>
		Component& GetUnique()
		{
			return registry.ctx<Component>();
		}

		template<typename Component>
		const Component* TryGetUnique() const
		{
			return registry.try_ctx<const Component>();
		}

		template<typename Component>
		Component* TryGetUnique()
		{
			return registry.try_ctx<Component>();
		}

		template<typename Component>
		bool HasUnique() const
		{
			return TryGetUnique<const Component>() != nullptr;
		}


		template<typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {}) const
		    -> View<TExclude<Exclude...>, std::add_const_t<Component>...>
		{
			return View<TExclude<Exclude...>, std::add_const_t<Component>...>{
			    registry.view<Component...>(excluded)};
		}

		template<typename... Component, typename... Exclude>
		auto MakeView(TExclude<Exclude...> excluded = {})
		    -> View<TExclude<Exclude...>, Component...>
		{
			return View<TExclude<Exclude...>, Component...>{registry.view<Component...>(excluded)};
		}

		template<typename Callback>
		void Each(Callback cb) const
		{
			registry.each(cb);
		}

		template<typename Callback>
		void EachOrphan(Callback cb) const
		{
			registry.orphans(cb);
		}

		template<typename... Components>
		void Clear()
		{
			registry.clear<Components...>();
		}

		void Reset()
		{
			registry = {};
			CachePools();
		}

		View<TExclude<>, CParent>& GetParentView()
		{
			return *parentView;
		}

		View<TExclude<>, CChild>& GetChildView()
		{
			return *childView;
		}

		const View<TExclude<>, CParent>& GetParentView() const
		{
			return *parentView;
		}

		const View<TExclude<>, CChild>& GetChildView() const
		{
			return *childView;
		}

		template<typename Component>
		auto OnConstruct()
		{
			return registry.on_construct<Component>();
		}

		template<typename Component>
		auto OnDestroy()
		{
			return registry.on_destroy<Component>();
		}

		template<typename Component>
		auto OnUpdate()
		{
			return registry.on_update<Component>();
		}

		const NativeTypeIds& GetNativeTypes() const
		{
			return nativeTypes;
		}

		Registry& GetRegistry()
		{
			return registry;
		}
		const Registry& GetRegistry() const
		{
			return registry;
		}

		template<typename Component>
		AST::Id GetFirstId() const
		{
			auto view = MakeView<Component>();
			if (view.Size() > 0)
			{
				return *view.begin();
			}
			return AST::NoId;
		}

		// Finds or creates a pool
		template<typename T>
		TPool<T>& AssurePool() const;
		BasePool* FindPool(Refl::TypeId componentId) const;

#pragma endregion ECS API

		void CopyFrom(const Tree& other);

	private:
		void SetupNativeTypes();
		void CachePools();
	};

	template<typename T>
	inline TPool<T>& Tree::AssurePool() const
	{
		constexpr Refl::TypeId componentId = Refl::TypeId::Get<T>();

		const TPair<i32, bool> result = pools.FindOrAddSorted({componentId});
		PoolInstance& pool            = pools[result.first];
		if (result.second)    // Added a new pool
		{
			pool.instance = MakeUnique<TPool<T>>());
			// TODO: Static inheritance methods
		}
		return *static_cast<TPool<T>*>(pool.instance.Get());
	}

	inline BasePool* Tree::FindPool(Refl::TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId});
		return index != NO_INDEX ? pools[index].instance.Get() : nullptr;
	}
}    // namespace Rift::AST
