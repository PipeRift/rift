// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Access.h"
#include "AST/Components/CChild.h"
#include "AST/Components/CParent.h"
#include "AST/Filter.h"
#include "AST/IdRegistry.h"
#include "AST/Pool.h"
#include "AST/Types.h"

#include <Memory/UniquePtr.h>
#include <Strings/Name.h>

#include <any>


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
	private:
		IdRegistry idRegistry;
		mutable TArray<PoolInstance> pools;
		TArray<OwnPtr> statics;

		NativeTypeIds nativeTypes;
		TOwnPtr<TFilter<Access::In<CParent>>> parentView;
		TOwnPtr<TFilter<Access::In<CChild>>> childView;


	public:
		Tree();
		explicit Tree(const Tree& other) noexcept;
		explicit Tree(Tree&& other) noexcept;
		Tree& operator=(const Tree& other) noexcept;
		Tree& operator=(Tree&& other) noexcept;

#pragma region ECS API
		Id Create();
		void Create(Id id);
		void Create(TSpan<Id> ids);
		void Destroy(Id id);
		void Destroy(TSpan<const Id> ids);

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
		decltype(auto) Add(TSpan<const Id> ids, const Component& value = {})
		{
			return AssurePool<Component>().Add(ids.begin(), ids.end(), value);
		}

		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename... Component>
		void Add(TSpan<const Id> ids) requires(sizeof...(Component) > 1)
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
		void Remove(TSpan<const Id> ids) requires(sizeof...(Component) > 0)
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
			return std::forward_as_tuple(Get<Component>(id)...);
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
			return std::forward_as_tuple(TryGet<Component>(id)...);
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
			for (const auto& instance : pools)
			{
				if (instance.GetPool()->Has(id))
				{
					return false;
				}
			}
			return true;
		}

		template<typename Static>
		Static& SetStatic(Static&& value = {})
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();

			// Find static first to replace it
			i32 index = statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				// Found, replace instance
				OwnPtr& instance = statics[index];
				instance         = MakeOwned<Static>(Forward<Static>(value));
				return *instance.GetUnsafe<Static>();
			}

			// Not found. return new instance
			index = statics.AddSorted<SortLessStatics>(MakeOwned<Static>(Forward<Static>(value)));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static>
		Static& SetStatic(const Static& value)
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();

			// Find static first to replace it
			i32 index = statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				// Found, replace instance
				OwnPtr& instance = statics[index];
				instance         = MakeOwned<Static>(value);
				return *instance.GetUnsafe<Static>();
			}

			// Not found. return new instance
			index = statics.AddSorted<SortLessStatics>(MakeOwned<Static>(value));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static>
		Static& GetOrSetStatic(Static&& newValue = {})
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();
			i32 index                 = statics.LowerBound<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				if (typeId != statics[index].GetType())
				{
					// Not found, insert sorted
					statics.Insert(index, MakeOwned<Static>(Forward<Static>(newValue)));
				}
				return *statics[index].GetUnsafe<Static>();
			}
			// Not found, insert sorted
			index =
			    statics.AddSorted<SortLessStatics>(MakeOwned<Static>(Forward<Static>(newValue)));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static>
		Static& GetOrSetStatic(const Static& newValue)
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();
			i32 index                 = statics.LowerBound<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				if (typeId != statics[index].GetType())
				{
					// Not found, insert sorted
					statics.Insert(index, MakeOwned<Static>(newValue));
				}
				return *statics[index].GetUnsafe<Static>();
			}
			// Not found, insert sorted
			index = statics.AddSorted<SortLessStatics>(MakeOwned<Static>(newValue));
			return *statics[index].GetUnsafe<Static>();
		}

		template<typename Static>
		bool RemoveStatic()
		{
			const Refl::TypeId typeId = Refl::TypeId::Get<Static>();
			const i32 index = statics.FindSortedEqual<Refl::TypeId, SortLessStatics>(typeId);
			if (index != NO_INDEX)
			{
				statics.RemoveAt(index);
				return true;
			}
			return false;
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
		TAccess<TInclude<Component...>, TExclude<Exclude...>> Access(
		    TExclude<Exclude...> = {}) const
		{
			return {{&AssurePool<Component>()...}, {&AssurePool<Exclude>()...}};
		}


		template<typename... Component, typename... Exclude>
		TFilter<TAccess<TInclude<Const<Component>...>, TExclude<Exclude...>>> Filter(
		    TExclude<Exclude...> = {}) const
		{
			static_assert(sizeof...(Component) > 0, "Exclusion-only filters are not supported");
			return {Access<Const<Component>...>(TExclude<Exclude...>{})};
		}

		template<typename... Component, typename... Exclude>
		TFilter<TAccess<TInclude<Component...>, TExclude<Exclude...>>> Filter(
		    TExclude<Exclude...> = {})
		{
			static_assert(sizeof...(Component) > 0, "Exclusion-only filters are not supported");
			return {Access<Component...>(TExclude<Exclude...>{})};
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

		template<typename... Component>
		void Clear()
		{
			(ClearPool<Component>(), ...);
		}

		void Reset()
		{
			idRegistry = {};
			pools.Empty();
			CachePools();
		}

		TFilter<Access::In<CParent>>& GetParentView()
		{
			return *parentView;
		}

		TFilter<Access::In<CChild>>& GetChildView()
		{
			return *childView;
		}

		const TFilter<Access::In<CParent>>& GetParentView() const
		{
			return *parentView;
		}

		const TFilter<Access::In<CChild>>& GetChildView() const
		{
			return *childView;
		}

		template<typename Component>
		TBroadcast<Tree&, TSpan<const Id>>& OnAdd()
		{
			return AssurePool<Component>().OnAdd();
		}

		template<typename Component>
		TBroadcast<Tree&, TSpan<const Id>>& OnRemove()
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
			auto view = Filter<Component>();
			if (view.Size() > 0)
			{
				return *view.begin();
			}
			return AST::NoId;
		}

		// Finds or creates a pool
		template<typename T>
		TPool<Mut<T>>& AssurePool() const;

		Pool* FindPool(Refl::TypeId componentId) const;

		template<typename T>
		CopyConst<TPool<Mut<T>>, T>* FindPool() const
		{
			return static_cast<CopyConst<TPool<Mut<T>>, T>*>(FindPool(Refl::TypeId::Get<Mut<T>>()));
		}

#pragma endregion ECS API
	private:
		void CopyFrom(const Tree& other);
		void MoveFrom(Tree&& other);

		void SetupNativeTypes();
		void CachePools();

		template<typename Component>
		void ClearPool()
		{
			if (auto* pool = FindPool<Component>())
			{
				pool->Reset();
			}
		}

		template<typename T>
		PoolInstance CreatePoolInstance() const;
	};

	template<typename T>
	inline TPool<Mut<T>>& Tree::AssurePool() const
	{
		constexpr Refl::TypeId componentId = Refl::TypeId::Get<Mut<T>>();

		i32 index = pools.LowerBound(componentId);
		if (index != NO_INDEX)
		{
			if (componentId != pools[index].GetId())
			{
				pools.Insert(index, CreatePoolInstance<T>());
			}
		}
		else
		{
			index = pools.Add(CreatePoolInstance<T>());
		}

		Pool* pool = pools[index].GetPool();
		return *static_cast<TPool<Mut<T>>*>(pool);
	}

	template<typename T>
	inline PoolInstance Tree::CreatePoolInstance() const
	{
		constexpr Refl::TypeId componentId = Refl::TypeId::Get<Mut<T>>();

		Tree& self = const_cast<Tree&>(*this);
		PoolInstance instance{componentId};
		instance.pool = Move(MakeOwned<TPool<Mut<T>>>(self));
		return Move(instance);
	}
}    // namespace Rift::AST
