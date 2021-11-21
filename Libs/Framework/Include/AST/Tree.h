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

#include <any>
#include <entt/entity/registry.hpp>


namespace Rift::AST
{
	struct SortLessStatics
	{
		constexpr bool operator()(const OwnPtr& a, const OwnPtr& b) const
		{
			return a.GetType() < b.GetType();
		}

		constexpr bool operator()(Refl::TypeId a, const OwnPtr& b) const
		{
			return a < b.GetType();
		}

		constexpr bool operator()(const OwnPtr& a, Refl::TypeId b) const
		{
			return a.GetType() < b;
		}
	};


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

	private:
		IdRegistry idRegistry;
		mutable TArray<PoolInstance> pools;
		TOwnPtr<TQuery<TExclude<>, CChild>> childView;
		TOwnPtr<TQuery<TExclude<>, CParent>> parentView;
		NativeTypeIds nativeTypes;
		TArray<OwnPtr> statics;


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

		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename Component>
		decltype(auto) Add(Id id, Component&& value = {})
		{
			Check(IsValid(id));
			return AssurePool<Component>().Add(id, Forward<Component>(value));
		}
		template<typename Component>
		decltype(auto) Add(Id id, const Component& value)
		{
			Check(IsValid(id));
			return AssurePool<Component>().Add(id, value);
		}

		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename... Component>
		void Add(Id id) requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			(Add<Component>(id), ...);
		}

		template<typename Component>
		decltype(auto) Add(TArrayView<const Id> ids, const Component& value = {})
		{
			return AssurePool<Component>().Add(ids.begin(), ids.end(), value);
		}

		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename... Component>
		void Add(TArrayView<const Id> ids) requires(sizeof...(Component) > 1)
		{
			(Add<Component>(ids), ...);
		}

		template<typename Component>
		Component& GetOrAdd(Id id)
		{
			Check(IsValid(id));
			return AssurePool<Component>().GetOrAdd(id);
		}


		template<typename Component>
		void Remove(const Id id)
		{
			FindPool<Component>()->Remove(id);
		}
		template<typename... Component>
		void Remove(const Id id) requires(sizeof...(Component) > 1)
		{
			(FindPool<Component>()->Remove(id), ...);
		}
		template<typename... Component>
		void Remove(TArrayView<const Id> ids) requires(sizeof...(Component) > 0)
		{
			for (Id id : ids)
			{
				Check(IsValid(id));
			}
			(FindPool<Component>()->Remove(ids), ...);
		}

		template<typename Component>
		Component& Get(const Id id) const
		{
			Check(IsValid(id));
			auto* const pool = FindPool<Component>();
			Check(pool);
			return pool->Get(id);
		}
		template<typename... Component>
		TTuple<Component&...> Get(const Id id) const requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			return std::forward_as_tuple(Get<Component>(id), ...);
		}
		template<typename Component>
		Component* TryGet(const Id id) const
		{
			auto* const pool = FindPool<Component>();
			return pool ? pool->TryGet(id) : nullptr;
		}
		template<typename... Component>
		TTuple<Component*...> TryGet(const Id id) const requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			return std::forward_as_tuple(TryGet<Component>(id), ...);
		}


		template<typename... Component>
		bool HasAny(Id id) const
		{
			return [id](const auto*... cpool) {
				return ((cpool && cpool->Has(id)) || ...);
			}(FindPool<Component>()...);
		}

		template<typename... Component>
		bool HasAll(Id id) const
		{
			return [id](const auto*... cpool) {
				return ((cpool && cpool->Has(id)) && ...);
			}(FindPool<Component>()...);
		}

		template<typename Component>
		bool Has(Id id) const
		{
			return HasAny<Component>(id);
		}

		bool IsValid(Id id) const
		{
			return idRegistry.IsValid(id);
		}

		bool IsOrphan(const Id id) const
		{
			for (const auto& pool : pools)
			{
				if (pool.GetInstance()->Has(id))
				{
					return false;
				}
			}
			return true;
		}

		template<typename Static, typename... Args>
		Static& SetStatic(Args&&... args)
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();

			// Find static first to replace it
			i32 index = statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				// Found, replace instance
				OwnPtr& instance = statics[index];
				instance         = MakeOwned<Static>(Forward<Args>(args)...);
				return *instance.GetUnsafe<Static>();
			}

			// Not found. return new instance
			index = statics.AddSorted<SortLessStatics>(MakeOwned<Static>(Forward<Args>(args)...));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static, typename... Args>
		Static& GetOrSetStatic(Args&&... args)
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();
			i32 index                 = statics.LowerBound<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				if (typeId != statics[index].GetType())
				{
					// Not found, insert sorted
					statics.Insert(index, MakeOwned<Static>(Forward<Args>(args)...));
				}
				return *statics[index].GetUnsafe<Static>();
			}
			// Not found, insert sorted
			index = statics.AddSorted<SortLessStatics>(MakeOwned<Static>(Forward<Args>(args)...));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static>
		Static& GetStatic()
		{
			return *TryGetStatic<Static>();
		}
		template<typename Static>
		const Static& GetStatic() const
		{
			return *TryGetStatic<Static>();
		}

		template<typename Static>
		Static* TryGetStatic()
		{
			const i32 index =
			    statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(Refl::TypeId::Get<Static>());
			return index != NO_INDEX ? statics[index].GetUnsafe<Static>() : nullptr;
		}
		template<typename Static>
		const Static* TryGetStatic() const
		{
			const i32 index =
			    statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(Refl::TypeId::Get<Static>());
			return index != NO_INDEX ? statics[index].GetUnsafe<Static>() : nullptr;
		}

		template<typename Static>
		bool HasStatic() const
		{
			const i32 index =
			    statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(Refl::TypeId::Get<Static>());
			return index != NO_INDEX;
		}


		template<typename... Component, typename... Exclude>
		TQuery<TExclude<Exclude...>, std::add_const_t<Component>...> Query(
		    TExclude<Exclude...> = {}) const
		{
			static_assert(sizeof...(Component) > 0, "Exclusion-only views are not supported");
			return {
			    {&AssurePool<std::remove_const_t<Component>>()...}, {&AssurePool<Exclude>()...}};
		}

		template<typename... Component, typename... Exclude>
		TQuery<TExclude<Exclude...>, Component...> Query(TExclude<Exclude...> = {})
		{
			static_assert(sizeof...(Component) > 0, "Exclusion-only views are not supported");
			return {
			    {&AssurePool<std::remove_const_t<Component>>()...}, {&AssurePool<Exclude>()...}};
		}

		template<typename Callback>
		void Each(Callback cb) const
		{
			idRegistry.Each(cb);
		}

		template<typename Callback>
		void EachOrphan(Callback cb) const
		{
			Each([this, &cb](const Id id) {
				if (IsOrphan(id))
				{
					cb(id);
				}
			});
		}

		template<typename... Components>
		void Clear()
		{
			registry.clear<Components...>();
		}

		void Reset()
		{
			idRegistry = {};
			pools.Empty();
			CachePools();
		}

		TQuery<TExclude<>, CParent>& GetParentView()
		{
			return *parentView;
		}

		TQuery<TExclude<>, CChild>& GetChildView()
		{
			return *childView;
		}

		const TQuery<TExclude<>, CParent>& GetParentView() const
		{
			return *parentView;
		}

		const TQuery<TExclude<>, CChild>& GetChildView() const
		{
			return *childView;
		}

		template<typename Component>
		TBroadcast<TArrayView<const Id>> OnAdd()
		{
			return AssurePool<Component>().OnAdd();
		}

		template<typename Component>
		TBroadcast<TArrayView<const Id>>& OnRemove()
		{
			return AssurePool<Component>().OnRemove();
		}

		const NativeTypeIds& GetNativeTypes() const
		{
			return nativeTypes;
		}

		template<typename Component>
		AST::Id GetFirstId() const
		{
			auto view = Query<Component>();
			if (view.Size() > 0)
			{
				return *view.begin();
			}
			return AST::NoId;
		}

		// Finds or creates a pool
		template<typename T>
		TPool<T>& AssurePool() const;

		Pool* FindPool(Refl::TypeId componentId) const;

		template<typename T>
		TPool<T>* FindPool() const
		{
			return static_cast<TPool<T>*>(FindPool(Refl::TypeId::Get<T>()));
		}

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
			pool.Create<T>();
		}
		return *static_cast<TPool<T>*>(pool.GetInstance());
	}
}    // namespace Rift::AST
