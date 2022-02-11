// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Templates/TypeList.h>


namespace Rift::AST
{
	template<typename... T>
	struct TAccess
	{
		template<typename... K>
		friend struct TAccess;

		using Components    = TTypeList<T...>;
		using MutComponents = TTypeList<Mut<T>...>;

	private:
		Tree& ast;
		TTuple<TPool<Mut<T>>*...> pools;


	public:
		TAccess(Tree& ast) : ast{ast}, pools{&ast.AssurePool<T>()...} {}
		TAccess(const TAccess& other) : ast{other.ast}, pools{other.pools} {}

		// Construct a child access (super-set) from another access
		template<typename... T2>
		TAccess(const TAccess<T2...>& other)
		    : ast{other.ast}, pools{std::get<TPool<Mut<T>>*>(other.pools)...}
		{
			constexpr bool noDependenciesMissing =
			    (ListContains<typename TAccess<T2...>::MutComponents, Mut<T>>() && ...);
			static_assert(noDependenciesMissing,
			    "Parent TAccess lacks one or more dependencies from a child TAccess.");

			using Components2 = typename TAccess<T2...>::Components;
			constexpr bool noInvalidMutableDependencies =
			    ((ListContains<Components2, Mut<T>>() || ListContains<Components2, T>()) && ...);
			static_assert(noInvalidMutableDependencies,
			    "Parent TAccess lacks one or more *mutable* dependencies from a child TAccess.");
		}


		template<typename C>
		TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			constexpr bool canModify = ListContains<Components, Mut<C>>();
			static_assert(canModify, "Can't modify components of this type");

			return std::get<TPool<Mut<C>>*>(pools);
		}

		template<typename C>
		const TPool<Mut<C>>* GetPool() const requires(IsSame<C, const C>)
		{
			constexpr bool canRead =
			    ListContains<Components, Mut<C>>() || ListContains<Components, const C>();
			static_assert(canRead, "Can't read components of this type");

			return std::get<TPool<Mut<C>>*>(pools);
		}

		bool IsValid(Id id) const
		{
			return ast.IsValid(id);
		}


		bool Has(Id id) const
		{
			return !IsNone(id) && (Has<T>(id) && ...);
		}

		template<typename... C>
		bool Has(Id id) const
		{
			return (GetPool<const C>()->Has(id) && ...);
		}

		template<typename C>
		decltype(auto) Add(Id id, C&& value = {}) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(Id id, const C& value) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, value);
		}

		template<typename C>
		void Remove(const Id id) const requires(IsSame<C, Mut<C>>)
		{
			GetPool<C>()->Remove(id);
		}
		template<typename... C>
		void Remove(TSpan<const Id> ids) const requires(IsSame<C, Mut<C>>&&...)
		{
			(GetPool<C>()->Remove(ids), ...);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id) const
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		C& GetOrAdd(Id id) const requires(IsMutable<C>)
		{
			return GetPool<C>()->GetOrAdd(id);
		}

		i32 Size() const
		{
			static_assert(sizeof...(T) == 1, "Can only get the size of single component accesses");
			return GetPool<T...>()->Size();
		}

		AST::Tree& GetAST() const
		{
			return ast;
		}
	};


	struct Access
	{
	protected:

		TArray<TPair<Refl::TypeId, Pool*>> pools;
		Tree& ast;


	public:
		Access(Tree& ast) : ast{ast} {}
	};

	template<typename... T>
	using TAccessRef = const TAccess<T...>&;
}    // namespace Rift::AST
