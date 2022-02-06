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
		const Tree& ast;
		TTuple<TPool<Mut<T>>*...> pools;


	public:
		TAccess(const Tree& ast) : ast{ast}, pools{&ast.AssurePool<T>()...} {}
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


		bool Has(Id id) const
		{
			return (Has<T>(id) && ...);
		}

		template<typename... C>
		bool Has(Id id) const
		{
			return (GetPool<C>()->Has(id) && ...);
		}

		template<typename C>
		decltype(auto) Add(Id id, C&& value = {})
		{
			return GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(Id id, const C& value)
		{
			return GetPool<C>()->Add(id, value);
		}

		template<typename T>
		TPool<Mut<T>>* GetPool() const requires(IsSame<T, Mut<T>>)
		{
			constexpr bool canModify = ListContains<Components, Mut<T>>();
			static_assert(canModify, "Can't modify components of this type");

			return std::get<TPool<Mut<T>>*>(pools);
		}

		template<typename T>
		const TPool<Mut<T>>* GetPool() const requires(IsSame<T, const T>)
		{
			constexpr bool canRead =
			    ListContains<Components, Mut<T>>() || ListContains<Components, const T>();
			static_assert(canRead, "Can't read components of this type");

			return std::get<TPool<Mut<T>>*>(pools);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id)
		{
			return GetPool<C>()->TryGet(id);
		}

		i32 Size() const
		{
			static_assert(sizeof...(T) == 1, "Can only get the size of single component accesses");
			return GetPool<T...>()->Size();
		}

		const AST::Tree& GetAST() const
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
}    // namespace Rift::AST
